<?php
class geolocation{
	private $mapquest_api;

	function __construct(){
		$this->mapquest_api = $this->getApi('mapquest');
	}

	function __destruct(){
		db_set_active('default');
	}

	protected function ipCheck($ip){
		if($ip == '::1'){
			$ip = '24.19.187.86';
		}
		return $ip;
	}

	protected function getApi($name){
		db_set_active('default');
		$q = db_query("select * from geolocation_api_values where name = :name", array(':name'=>$name));
		return $q->fetchObject();
	}

	protected function apiLimitCheck($obj){
		db_set_active('default');
		$timestamp = strtotime('-'.$obj->limit_time_unit);
		$queryArr = array(
			':api_val_id'=>$obj->id,
			':timestamp'=>$timestamp,
		);
		$query = "select sum(insert_count) as used from geolocation_api_track where api_val_id = '".$obj->id."' and timestamp >= '".$timestamp."'";
		$q = db_query($query);
		//$q = db_query("select count(insert_count) as used from geolocation_api_track where api_val_id = ':api_val_id' and timestamp >= ':timestamp'", $queryArr);
		$r = $q->fetchAssoc();
		$output['value'] = $obj->limit_qty - $r['used'];
		$output['query'] = $query;
		return $output;
	}

	protected function getIpv4($ip){
		$ipArr = explode('.', $ip);
		$output =  (16777216 * $ipArr[0])+(65536 * $ipArr[1]) + (256 * $ipArr[2])+ $ipArr[3];
		return $output;
	}

	protected function getIp($ipAddress, $json = true){
		$output = array();
		db_set_active('geolocation');
		$q = db_query("select loc_id from geolocation.ip_locations where INET_ATON(:ipa) between start_ip_num and end_ip_num limit 1", array(':ipa'=>$this->ipCheck($ipAddress)));
		if(!$q){
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		else{
			$r = $q->fetchAssoc();
			$locId = $r['loc_id'];
			$query2 = "select * from geolocation.cities where id = '".$locId."'";
			$q2 = db_query($query2);
			if($q2){
				$r2 = $q2->fetchAssoc();
				$output['status'] = 'success';
				$output['results'] = $r2;
			}
			
		}
		if($json){
			$output = json_encode($output);
		}
		return $output;

		
	}

	protected function getPostalCode($postalCode, $json = true){
		$output = array();
		db_set_active('geolocation');
		$q = db_query("select * from geolocation.cities where postal_code = :pc", array(':pc'=>$postalCode));
		if($q){
			$r = $q->fetchAssoc();
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function getCityPostalByGeo($val, $json = true){
		$coords = explode(',', $val);
		$lat = $coords[0];
		$lng = $coords[1];
		db_set_active('geolocation');
		$query = "select *, round(degrees(acos(sin(radians(:lat))*sin(radians(lat))+cos(radians(:lat))*cos(radians(lat))*cos(radians(:lng - lng)))) *69.09) as distance from geolocation.cities  where postal_code != '' order by distance asc limit 1";
		$insertArr = array(
			':lat'=>$lat,
			':lng'=>$lng,
		);
		$q = db_query($query, $insertArr);
		if($q){
			$r = $q->fetchAssoc();
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function getAddressByGeo($geo, $isJson = true){
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?latlng='.$geo.'&sensor=false';
		$json = json_decode(file_get_contents($url), true);
		$address = explode(',', $json['results'][0]['formatted_address']);
		$results['address'] = trim($address[0]);
		$results['city'] = trim($address[1]);
		$stateZip = explode(' ', trim($address[2]));
		$results['state_region'] = $stateZip[0];
		$results['postal_code'] = $stateZip[1];
		$results['country'] = trim($address[3]);
		$output['status'] = 'success';
		$output['results'] = $results;
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function getGeoByAddress($address, $isJson = true){
		$address = urlencode($address);
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?address='.$address.'&sensor=false';
		$json = json_decode(file_get_contents($url), true);
		$results['lat'] = $json['results'][0]['geometry']['location']['lat'];
		$results['lng'] = $json['results'][0]['geometry']['location']['lng'];
		$output['status'] = 'success';
		$output['results'] = $results;
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function getGeoByAddressMq($address, $isJson = true){
		$results = array();
		$count = count($address);
		$limit = $this->apiLimitCheck($this->mapquest_api);
		if($limit['value'] < $count){
			$output['msg'] = 'Notice: '.$limit.' of the '.$count.' records without lat/lon values were updated due to API limitations.<br />';
			$count = $limit['value'];
		}
		elseif($limit['value'] > $count){
			$remainder = $limit['value'] - $count;
			if($remainder > 1000){
				$output['msg'] = 'Notice: there are '.$remainder.' API calls remaining for this API\'s limitation settings.  Contact an admin for more information.<br />';
			}
		}
		$results = array();
		$batchNumber = ceil($count/100);
		for($i=0; $i<$batchNumber; $i++){
			$start = $i*100;
			if($i+1 == $batchNumber){
				$end = $count;
			}
			else{
				$end = $start+100;
			}
			$location = '';
			for($m=$start; $m<$end; $m++){
				$location .= '&location='.urlencode($address[$m]['address']);
			}
			$url = 'http://www.mapquestapi.com/geocoding/v1/batch?key='.$this->mapquest_api->api_key.'&callback=renderBatch'.$location;
			$json = file_get_contents($url);
			$json = str_replace('renderBatch(', '', $json);
			$json = substr($json, 0, -2);
			$arr = json_decode($json, true);
			$rCount = count($arr['results']);
			for($r=0; $r<$rCount; $r++){
				$indexAt = $start+$r;
				$mqResults['index'] = $address[$indexAt]['index'];
				$mqResults['lat'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lat'];
				$mqResults['lng'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lng'];
				$results[] = $mqResults;
			}
		}
		$insert = db_insert('geolocation_api_track')
			->fields(array(
				'api_val_id'=>$this->mapquest_api->id,
				'insert_count'=>$count,
				'timestamp'=>REQUEST_TIME
			))
			->execute();
		$output['status'] = 'success';
		$output['results'] = $results;
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function getStores($request, $isJson = true){
		db_set_active('phad_maps');
		$lat = '';
		$lng = '';
		if(isset($request['value'])){
			$geo = explode(',', $request['value']);
			$lat = $geo[0];
			$lng = $geo[1];
		}
		else{
			$lng = $request['lng'];
			$lat = $request['lat'];
		}
		

		//Default distance is 30 miles, and can be overwritten by the request value.
		$dist = 30;
		if(isset($request['dist'])){
			$dist = $request['dist'];
		}
		//The next four lines draw up the latitude/longitude permiter to search in, and modified for PHP.  It uses the values from $lat, $lng, and $dist to perform the calculations.  It is based on that 1 degree of latitude is equivelant to 69.09 miles and 1 degree of longitude is equivelant to the cosign of the latitude times 69.09.  For more detailed information, see slides 10 and 11 of the GeoSpacial MySQL Search whitepaper at http://www.scribd.com/doc/2569355/Geo-Distance-Search-with-MySQL.
		$lng1 = $lng-$dist/abs(cos(deg2rad($lat))*69.09);
		$lng2 = $lng+$dist/abs(cos(deg2rad($lat))*69.09);
		$lat1 = $lat-($dist/69.09);
		$lat2 = $lat+($dist/69.09);
		//Default campaign ID is 7698 (Naked Juice), but can be overwritten by the request value.
		$campaignId = 7698;
		if(isset($request['campaign_id'])){
			$campaignId = $request['campaign_id'];
		}
		//Output array is set
		$output = array();
		//Query is set and run
		$insertVals = array(
			':lat'=>$lat,
			':lng'=>$lng,
			':lat1'=>$lat1,
			':lat2'=>$lat2,
			':lng1'=>$lng1,
			':lng2'=>$lng2,
			':campaignId'=>$campaignId,
		);
		$query = "select *, round(degrees(acos(sin(radians(:lat))*sin(radians(lat))+cos(radians(:lat))*cos(radians(lat))*cos(radians(:lng - lng)))) *69.09) as distance from phad_maps.locations where lng between :lng1 and :lng2 and lat between :lat1 and :lat2 and campaignID = :campaignId order by distance";
		if(isset($request['limit']) && is_numeric($request['limit']) && $request['limit'] >= 1){
			$query .= " limit ".$request['limit'];
		}
		$q = db_query($query, $insertVals);
		//Outputs an error message if there was a problem with the query
		if(!$q){
			$output['status'] = 'error';
			$output['error_msg'] = db_error();
			/*
			if(isset($request['debug'])){
				$output['error_msg'] = $db->error;
			}
			*/
		}
		//If query is successful, results are outputted.
		else{
			$output['status'] = 'success';
			/*
			$output['msg'] = 'count is '.$q->rowCount().', distance is '.$dist;
			foreach($insertVals as $k=>$v){
				$query = str_replace($k, $v, $query);
			}
			$output['query'] = $query;
			*/
			while($r = $q->fetchAssoc()){
				$output['results'][] = $r;
			}
			
		}
		//Output is converted into JSON by default, unless otherwise specified.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
		db_set_active('default');
	}

	protected function getWeather($request, $isJson = true){
		$output = array();
		switch($request['subtype']){
			case 'postal_code':
				$results = $this->getPostalCode($request['value'], false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
				break;
			case 'ip_address':
				$results = $this->getIp($request['value'], false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
				break;
			default:
				$geo = explode(',', $request['value']);
				$results = $this->getCityPostalByGeo($request['value'], false);
				$lat = $geo[0];
				$lng = $geo[1];
		}
		$start = REQUEST_TIME;
		$end = $start+(60*60);
		$timestamp = $start-(60*60*3);
		db_set_active('default');
		$qArray = array(':postal_code'=>$results['results']['postal_code'],':timestamp'=>$timestamp);
		$q = db_query('select * from geolocation_weather_cache where postal_code = :postal_code and timestamp >= :timestamp order by timestamp desc limit 1', $qArray);
		$rowCount = $q->rowCount();
		if($rowCount > 0){
			$remove = array('id', 'timestamp');
			$r = $q->fetchAssoc();
			foreach($r as $k=>$v){
				if(in_array($k, $remove)){
					unset($r[$k]);
				}
			}
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			db_query('delete from geolocation_weather_cache where postal_code = :postal_code and timestamp < :timestamp', $qArray);
			$timeStart = date('Y-m-d', $start).'T'.date('H:i:s', $start);
			$timeEnd = date('Y-m-d', $end).'T'.date('H:i:s', $end);
			$nwsLink = 'http://graphical.weather.gov/xml/sample_products/browser_interface/ndfdXMLclient.php?lat='.$lat.'&lon='.$lng.'&product=time-series&begin='.$timeStart.'&end='.$timeEnd.'&Unit=e&maxt=maxt&mint=mint&temp=temp&pop12=pop12&qpf=qpf&rh=rh&sky=sky&wspd=wspd&wdir=wdir&wx=wx&icons=icons';
			$xml = simplexml_load_string(file_get_contents($nwsLink));
			if($xml){
				$output['status'] = 'success';
				$output['results']['city'] = $results['results']['city'];
				$output['results']['state_region'] = $results['results']['state_region'];
				$output['results']['postal_code'] = $results['results']['postal_code'];
				$output['results']['lat'] = $lat;
				$output['results']['lng'] = $lng;
				$output['results']['nws_xml'] = $nwsLink;
				$output['results']['nws_page'] = $xml->data->moreWeatherInformation;
				foreach($xml->children() as $data){
					foreach($data->parameters as $params){
						$output['results']['icon'] = $params->{'conditions-icon'}->{'icon-link'};
						$summary = array('coverage', 'intensity', 'weather-type');
						foreach($params->weather->{'weather-conditions'}->value as $value){
							foreach ($value->attributes() as $a=>$b){
								if(in_array($a, $summary)){
									$output['results']['weather_conditions'] .= $b.' ';
								}
							}
						}
						foreach($params->temperature as $temp){
							$found = false;
							foreach($temp->attributes() as $a=>$b){
								if($a == "type" && $b == "hourly"){
									$found = true;
									break;
								}
							}
							if($found){
								$output['results']['current_temp'] = $temp->value.'&deg;F';
								break;
							}
						}
						$output['results']['precipitation'] = $params->precipitation->value.' inches';
						$output['results']['cloud_cover'] = $params->{'cloud-amount'}->value.'%';
						$output['results']['12_hour_precip_prob'] = $params->{'probability-of-precipitation'}->value.'%';
						$output['results']['humidity'] = $params->humidity->value.'%';
						$output['results']['wind_dir'] = $this->degreeToDirection($params->direction->value);
						$output['results']['wind_speed'] = $this->knotsToMiles($params->{'wind-speed'}->value).' MPH';
					}
				}
				$fields = array();
				foreach($output['results'] as $k=>$v){
					$fields[] = $k;
				}
				$fields[] = 'timestamp';
				$values = $output['results'];
				$values['timestamp'] = REQUEST_TIME;
				db_insert('geolocation_weather_cache')->fields($fields)->values($values)->execute();
				$output['msg'] = 'Got from NWS directly';
			}
			else{
				$output['status'] = 'error';
				$output['msg'] = 'An error occred when attempting to retreive the weather information.  Please notify an administrator';
			}
		}
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function degreeToDirection($degree){
		if($degree > 348.75 || $degree <= 11.25){
			return 'N';
		}
		if($degree > 11.25 && $degree <= 33.75){
			return 'NNE';
		}
		if($degree > 33.75 && $degree <= 56.25){
			return 'NE';
		}
		if($degree > 56.25 && $degree <= 78.75){
			return 'ENE';
		}
		if($degree > 78.75 && $degree <= 101.25){
			return 'E';
		}
		if($degree > 101.25 && $degree <= 123.75){
			return 'ESE';
		}
		if($degree > 123.75 && $degree <= 146.25){
			return 'SE';
		}
		if($degree > 146.25 && $degree <= 168.75){
			return 'SSE';
		}
		if($degree > 168.75 && $degree <= 191.25){
			return 'S';
		}
		if($degree > 191.25 && $degree <= 213.75){
			return 'SSW';
		}
		if($degree > 213.75 && $degree <= 236.25){
			return 'SW';
		}
		if($degree > 236.25 && $degree <= 258.75){
			return 'WSW';
		}
		if($degree > 258.75 && $degree <= 281.25){
			return 'W';
		}
		if($degree > 281.25 && $degree <= 303.75){
			return 'WNW';
		}
		if($degree > 303.75 && $degree <= 326.25){
			return 'NW';
		}
		if($degree > 326.25 && $degree <= 348.75){
			return 'NW';
		}
	}

	protected function knotsToMiles($speed){
		return round(($speed*1.15078), 2);
	}

	protected function error($json = true){
		$output['status'] = 'error';
		$output['msg'] = 'invalid request';
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}

	protected function csvConvert($theVal, $valPosition){
		$csvTerminated = "\n";
	    $csvSeparator = ",";
	    $csvEnclosed = '"';
	    $csvEscaped = "\\";
		$valOutput = stripslashes(stripslashes($theVal));
		$valOutput = str_replace($csvEnclosed, $csvEscaped . $csvEnclosed, $valOutput);
		$valOutput = $csvEnclosed . $valOutput . $csvEnclosed;
		if($valPosition == "false"){
			$valOutput .= $csvSeparator;
		}
		else{
			$valOutput .= $csvTerminated;
		}
		return $valOutput;
	}

	public function menu(){
		$items['geolocation/ip_address'] = array(
			'title'=>'IP Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_ip_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/postal_code'] = array(
			'title'=>'Postal Code Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_pc_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/lat_lon_city'] = array(
			'title'=>'City Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_latlon1_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/lat_lon_address'] = array(
			'title'=>'Reverse Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_latlon2_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/address'] = array(
			'title'=>'Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_address_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/location_uploads'] = array(
			'title'=>'Location Uploads',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_latlon3_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		$items['geolocation/location_exports'] = array(
			'title'=>'Location Downloads Exporter',
			'type'=>MENU_CALLBACK,
			'page callback'=>'geolocation_export2',
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
		);
		$items['geolocation/location_downloads'] = array(
			'title'=>'Location Downloads',
			'type'=>MENU_CALLBACK,
			'page callback'=>'geolocation_downloads',
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
		);
		$items['geolocation/export'] = array(
			'title'=>'Geolocation Export',
			'type'=>MENU_CALLBACK,
			'page callback'=>'geolocation_export1',
			'access arguments'=>array('access content'),
		);
		$items['geolocation/weather'] = array(
			'title'=>'Geolocation Weather',
			'type'=>MENU_CALLBACK,
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_weather_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
		);
		return t($items);
	}

	public function permission(){
		return array(
	    	'access geolocation content' => array(
	     		'title' => t('Access Geolocation Content'),
	    	)
	  	);
	}

	public function ipForm($form, $form_state){
		$ipAddress = $_SERVER['REMOTE_ADDR'];
		$form['ip_address'] = array(
			'#type'=>'textfield',
			'#title'=>'IP Address',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/*
	public function ipFormValidate($form, $form_state){
		if (($form_state['values']['ip_address'] == '')){
			form_set_error('ip_address', t('An IP Address is required.'));
		}
	}
	*/

	public function ipFormSubmit($form, $form_state){
		$ipAddress = $_SERVER['REMOTE_ADDR'];
		if($form_state['values']['ip_address'] != ''){
			$ipAddress = $form_state['values']['ip_address'];
		}
		$results = $this->getIp($ipAddress, false);
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=ip_address&value='.$ipAddress.'" target="_blank">Export URL</a>';
		drupal_set_message(t($output));
	}

	public function pcForm($form, $form_state){
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#required'=>TRUE,
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	public function pcFormValidate($form, $form_state){
		if (($form_state['values']['postal_code'] == '')){
			return form_set_error('postal_code', t('A Postal Code is required.'));
		}
	}

	public function pcFormSubmit($form, $form_state){
		$results = $this->getPostalCode($form_state['values']['postal_code'], false);
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=postal_code&value='.$form_state['values']['postal_code'].'" target="_blank">Export URL</a>';
		drupal_set_message(t($output));
	}

	public function latLon1Form($form, $form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	public function latLon1FormValidate($form, $form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	public function latLon1FormSubmit($form, $form_state){
		$latLon = $form_state['values']['lat'].','.$form_state['values']['lon'];
		$results = $this->getCityPostalByGeo($latLon, false);
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=city_postal_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		drupal_set_message(t($output));
	}

	public function latLon2Form($form, $form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	public function latLon2FormValidate($form, $form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	public function latLon2FormSubmit($form, $form_state){
		$latLon = $form_state['values']['lat'].','.$form_state['values']['lon'];
		$results = $this->getAddressByGeo($latLon, false);
		$output = $results['results']['address'].'<br />'.$results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=address_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		drupal_set_message(t($output));
	}

	public function latLon3Form($form, $form_state){
		$form['csv_file'] = array(
			'#type'=>'file',
			'#title'=>'CSV File',
			'#size'=>20,
			'#upload_validators'=>array('file_validate_extensions'=>array('csv')),
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		$form['#attributes'] = array('enctype' => "multipart/form-data");
		return t($form);
	}

	public function latLon3FormSubmit($form, $form_state){
		if(isset($form_state['values']['csv_file'])){
			$output = 'in submission, file set<br />';
			
			db_set_active('default');
			$modPath = drupal_get_path('module', 'geolocation');
			$dir = 'public://geolocation/temp_files/';
			//file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
			$file = file_save_upload('csv_file', array('file_validate_extensions'=>array('csv')), $dir, FILE_EXISTS_RENAME);
			
			$filename = $dir.$file->filename;
			$fileUrl = file_create_url($filename);
			ini_set('auto_detect_line_endings', true);
			$fOpen = fopen($fileUrl, 'r');
			$addressLookup = array();
			$fields = array();
			$values = array();
			$indexes = array(
				'lat'=>'',
				'lng'=>'',
				'street'=>'',
				'city'=>'',
				'state'=>'',
				'postal'=>'',
				'campaignID'=>'',
			);
			$exclude = array('id');
			$rewrite = false;
			$campaignId = '';
			while(!feof($fOpen)){
				$csv = fgetcsv($fOpen);
				$count = count($csv);
				if(empty($fields)){
					for($i=0; $i<$count; $i++){
						if(!in_array($csv[$i], $exclude)){
							$fields[] = '`'.$csv[$i].'`';
							foreach($indexes as $in=>$inn){
								if(stristr($csv[$i], $in) !== FALSE){
									$indexes[$in] = $i;
								}
							}
						}
					}
				}
				else{
					if($csv[$indexes['lng']] == '' || $csv[$index['lat']] == ''){
						$rewrite = true;
						$address = $csv[$indexes['street']].', '.$csv[$indexes['city']].', '.$csv[$indexes['state']].' '.$csv[$indexes['postal']];
						$addInsert['index'] = count($values);
						$addInsert['address'] = $address;
						$addressLookup[] = $addInsert;
					}
					$fCount = count($fields);
					$query = array();
					for($i=0; $i<$fCount; $i++){
						if(empty($csv[$i])){
							$csv[$i] = NULL;
						}
						$query[$fields[$i]] = $csv[$i];
						if($i == $indexes['campaignID'] && $campaignId == ''){
							$campaignId = $csv[$i];
						}
					}
					$values[] = $query;
				}
			}
			
			$addResults = $this->getGeoByAddressMq($addressLookup, false);
			if($addResults['msg'] != ''){
				$output .= $addResults['msg'];
			}
			foreach($addResults['results'] as $ar){
				$values[$ar['index']]['`lat`'] = $ar['lat'];
				$values[$ar['index']]['`lng`'] = $ar['lng'];
			}
			fclose($fOpen);
			db_set_active('phad_maps');
			//db_query('truncate table locations');
			$q = db_insert('phad_maps.locations')->fields($fields);
			foreach($values as $val){
				$q->values($val);
			}
			$q->execute();
			db_set_active('default');
			$output .= 'Information from the CSV file has been updated with latitude/longitude information and saved to the database.<br />';
			$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/location_exports?campaign_id='.$campaignId.'&file_name='.$file->filename.'" target="_blank">Download Updated File</a>';
			drupal_set_message(t($output));
			file_delete($file);
		}
	}

	public function addressForm($form, $form_state){
		$form['address'] = array(
			'#type'=>'textfield',
			'#title'=>'Address',
			'#size'=>20,
			'#required'=>true,
		);
		$form['city'] = array(
			'#type'=>'textfield',
			'#title'=>'City',
			'#required'=>true,
			'#size'=>20,
		);
		$form['state'] = array(
			'#type'=>'textfield',
			'#title'=>'State',
			'#required'=>true,
			'#size'=>5,
		);
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	public function addressFormValidate($form, $form_state){
		$checkFor = array('city', 'state');
		foreach($checkFor as $c){
			if (($form_state['values'][$c] == '')){
				form_set_error($c, t('The value for '.$c.' is required.'));
			}
		}
	}

	public function addressFormSubmit($form, $form_state){
		$address = str_replace(' ', '+', $form_state['values']['address']).',+'.$form_state['values']['city'].',+'.$form_state['values']['state'];
		if($form_state['values']['postal_code'] != ''){
			$address .= ',+'.$form_state['values']['postal_code'];
		}
		$results = $this->getGeoByAddress($address, false);
		$output = 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=geo_by_address&value='.$address.'">Export URL</a>';
		drupal_set_message(t($output));
	}

	public function weatherForm($form, $form_state){
		$form['description'] = array(
			'#type'=>'markup',
			'#title'=>'Instructions',
			'#markup'=>'Enter either Latitude/Longitude coordinates, a postal code, or an IP address for real time weather information.  Latitude/Longitude will take precidence over postal code, and postal code will take precidence over an IP address.  If no fields are entered, the system will pull information based on your current IP address.',
		);
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#size'=>20,
		);
		$form['lng'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#size'=>20,
		);
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#size'=>20,
		);
		$form['ip_address'] = array(
			'#type'=>'textfield',
			'#title'=>'IP Address',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	public function weatherFormSubmit($form, $form_state){
		$request = array();
		$output = '';
		if($form_state['values']['lat'] != '' && $form_state['values']['lng'] != ''){
			$request['subtype'] = 'latlng';
			$request['value'] = $form_state['values']['lat'].','.$form_state['values']['lng'];
		}
		elseif($form_state['values']['postal_code'] != ''){
			$request['subtype'] = 'postal_code';
			$request['value'] = $form_state['values']['postal_code'];
		}
		else{
			$request['subtype'] = 'ip_address';
			if($form_state['values']['ip_address'] != ''){
				$request['value'] = $form_state['values']['ip_address'];
			}
			else{
				$request['value'] = $_SERVER['REMOTE_ADDR'];
			}
		}

		$results = $this->getWeather($request, false);
		//$output .= $results['msg'];
		if($results['status'] = 'success'){
			$output .= '<h2>Weather Information For '.$results['results']['city'].', '.$results['results']['state_region'].'</h2>';
			$output .= '<p><a href="'.$GLOBALS['base_url'].'/geolocation/export?type=weather&subtype='.$request['subtype'].'&value='.$request['value'].'" target="_blank">Export URL</a><br />';
			$output.='<a href="'.$results['results']['nws_xml'].'" target="_blank">NWS XML Source</a><br />';
			$output.= '<a href="'.$results['results']['nws_page'].'" target="_blank">NWS Page</a></p>';
			$output .= '<img src="'.$results['results']['icon'].'" /><br />';
			$output .= 'Temperature: '.$results['results']['current_temp'].'<br />';
			$output .= 'Precipitation: '.$results['results']['precipitation'].'<br />';
			$output .= 'Cloud Cover: '.$results['results']['cloud_cover'].'<br />';
			$output .= '12 Hour Probability of Precipitation: '.$results['results']['12_hour_precip_prob'].'<br />';
			$output .= 'Humidity: '.$results['results']['humidity'].'<br />';
			$output .= 'Winds out of the '.$results['results']['wind_dir'].' at '.$results['results']['wind_speed'].'<br />';
		}
		else{
			$output .= $results['msg'];
		}
		drupal_set_message(t($output));
	}

	public function downloads(){
		db_set_active('phad_maps');
		$output = '';
		$q = db_query('select distinct `campaignID` from phad_maps.locations where `campaignID` is not null and `campaignID` != 0');
		$output .= '<ul>';
		while($r = $q->fetchAssoc()){
			$output .= '<li><a href="'.$GLOBALS['base_url'].'/geolocation/location_exports?campaign_id='.$r['campaignID'].'" target="_blank">'.$r['campaignID'].'</a></li>';
		}
		$output .= '</ul>';

		db_set_active('default');
		return t($output);
	}

	public function export1($request){
		if(isset($request['type'])){
			header("Access-Control-Allow-Origin: *");
			header('Access-Control-Allow-Methods', 'GET POST');
			switch($request['type']){
				case 'ip_address':
					if(!isset($request['value'])){
						echo $this->getIp($_SERVER['REMOTE_ADDR']);
					}
					else{
						echo $this->getIp($request['value']);
					}
					break;
				case 'postal_code':
					if(!isset($request['value'])){
						echo $this->error();
					}
					else{
						echo $this->getPostalCode($request['value']);
					}
					break;
				case 'geo_by_address':
					if(!isset($request['value'])){
						echo $this->error();
					}
					else{
						echo $this->getGeoByAddress($request['value']);
					}
					break;
				case 'address_by_geo':
					if(!isset($request['value'])){
						echo $this->error();
					}
					else{
						echo $this->getAddressByGeo($request['value']);
					}
					break;
				case 'city_postal_by_geo':
					if(!isset($request['value'])){
						echo $this->error();
					}
					else{
						echo $this->getCityPostalByGeo($request['value']);
					}
					break;
				case 'weather':
					if($request['subtype'] != 'ip_address' && $request['value'] == ''){
						echo $this->error();
						break;
					}
					if($request['subtype'] == 'ip_address' && !isset($request['value'])){
						$request['value'] = $_SERVER['REMOTE_ADDR'];
					}
					echo $this->getWeather($request);
					break;
				case 'store_locator':
					if($request['value'] == '' &&($request['lat'] == '' || $request['lng'] == '')){
						echo $this->error();
						break;
					}
					echo $this->getStores($request);
					break;
			}
			
		}
		else{
			echo $this->error();
		}
	}

	public function export2(){
		db_set_active('phad_maps');
		$query = 'select * from phad_maps.locations';
		$campaignId = '';
		if(isset($_REQUEST['campaign_id'])){
			$query .= ' where `campaignID` = :cid';
			$campaignId = array(':cid'=> $_REQUEST['campaign_id']);
		}
		$q = db_query($query, $campaignId);;
		
		$header = false;
		$output = '';
		while($r = $q->fetchAssoc()){
			$arrCount = count($r);
			if(!$header){
				$hCount = 1;
				foreach($r as $k=>$v){
					$last = "false";
					if($hCount == $arrCount){
						$last = "true";
					}
					$output .= $this->csvConvert($k, $last);
					$hCount++;
				}
				$header = true;
			}
			$count = 1;
			foreach($r as $k=>$v){
				$last = "false";
				if($count == $arrCount){
					$last = "true";
				}
				$output .= $this->csvConvert($v, $last);
				$count++;
			}
		}
		db_set_active('default');
		$filename = 'phad_maps.csv';
		if(isset($_REQUEST['file_name'])){
			$filename = $_REQUEST['file_name'];
		}
		header ('Content-Disposition: attachment; filename=' . $filename . ';') ;
        echo $output;
        exit;
	}
}