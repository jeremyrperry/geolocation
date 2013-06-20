<?php
class geolocation{
	private $mapquest_api;

	function __construct(){
		$this->mapquest_api = $this->getApi('mapquest');
	}

	function __destruct(){
		db_set_active('default');
	}

	private function getApi($name){
		db_set_active('default');
		$q = db_query("select * from geolocation_api_values where name = :name", array(':name'=>$name));
		return $q->fetchObject();
	}

	private function apiLimitCheck($obj){
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

	public function getIpv4($ip){
		$ipArr = explode('.', $ip);
		$output =  (16777216 * $ipArr[0])+(65536 * $ipArr[1]) + (256 * $ipArr[2])+ $ipArr[3];
		return $output;
	}

	public function getIp($ipAddress, $json = true){
		$output = array();
		db_set_active('geolocation');
		$q = db_query("select loc_id from ip_locations where INET_ATON(:ipa) between start_ip_num and end_ip_num limit 1", array(':ipa'=>$ipAddress));
		if(!$q){
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		else{
			$r = $q->fetchAssoc();
			$locId = $r['loc_id'];
			$query2 = "select * from cities where id = '".$locId."'";
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

	public function getPostalCode($postalCode, $json = true){
		$output = array();
		db_set_active('geolocation');
		$q = db_query("select * from cities where postal_code = :pc", array(':pc'=>$postalCode));
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

	public function getCityPostalByGeo($val, $json = true){
		$coords = explode(',', $val);
		$lat = $coords[0];
		$lng = $coords[1];
		db_set_active('geolocation');
		$query = "select *, round(degrees(acos(sin(radians(:lat))*sin(radians(lat))+cos(radians(:lat))*cos(radians(lat))*cos(radians(:lng - lng)))) *69.09) as distance from cities  where postal_code != '' order by distance asc limit 1";
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

	public function getAddressByGeo($geo, $isJson = true){
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

	public function getGeoByAddress($address, $isJson = true){
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

	public function getGeoByAddressMq($address, $isJson = true){
		$results = array();
		$count = count($address);
		$limit = $this->apiLimitCheck($this->mapquest_api);
		//$limit = 5000;
		$output['msg'] = 'limit qty is '.$this->mapquest_api->limit_qty.' per '.$this->mapquest_api->limit_time_unit.', used during limit period was '.$limit['value'].'<br />';
		$output['msg'] .= 'Current time is '.time().', date range check is '.strtotime('-'.$this->mapquest_api->limit_time_unit).'<br />';
		$output['msg'] .= 'query was '.$limit['query'].'<br />';
		if($limit['value'] < $count){
			$output['msg'] .= 'Notice: '.$limit.' of the '.$count.' records without lat/lon values were updated due to API limitations.<br />';
			$count = $limit['value'];
		}
		elseif($limit['value'] > $count){
			$remainder = $limit['value'] - $count;
			//if($remainder > 1000){
				$output['msg'] .= 'Notice: there are '.$remainder.' API calls remaining for this API\'s limitation settings.  Contact an admin for more information.<br />';
			//}
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

	public function error($json = true){
		$output['status'] = 'error';
		$output['msg'] = 'invalid request';
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}
}