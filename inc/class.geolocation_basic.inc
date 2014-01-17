<?php
/**
*This script houses the Geolocation Module class and contains the module's presentation logic.  This class extends and depends upon the geolocation_basic_abstract class.
*@link http://www.jeremyrperry.com
*@version 3.0
*@author Jeremy Perry jperry@phluant.com, jeremyrperry@gmail.com
*@package Phluant Mobile Drupal 7 Geolocation Module
*@todo Updated on 01-15-2014 to provide a cleaner separation of presentation and data abstraction logic.  The export function was also modified to allow a direct PHP callup for use within a website.
*
*/

class geolocation_basic extends geolocation_basic_abstract{
	/**
	*The addressForm function returns the Drupal-based form settings for the reverse geolocation by address lookup.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function address_form($form, &$form_state){
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

	/**
	*The addressFormValidate function ensures the necessary values are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if any of the required values are blank
	*/
	public function address_form_validate($form, &$form_state){
		$checkFor = array('address', 'city', 'state');
		$msg = '';
		foreach($checkFor as $c){
			if (($form_state['values'][$c] == '')){
				if($msg != ''){
					$msg .= '<br />';
				}
				$msg .= 'The '.$c.' value is required.';
			}
		}
		if($msg != ''){
			form_set_error($c, t('The value for '.$c.' is required.'));
		}
	}

	/**
	*The addressFormSubmit function processes the reverse geolocation lookup by address and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function address_form_submit($form, &$form_state){
		//The address is arranged in a format that is easy to be read by Google.
		$request['value'] = str_replace(' ', '+', $form_state['values']['address']).',+'.$form_state['values']['city'].',+'.$form_state['values']['state'];
		//Postal code is added if filled out.
		if($form_state['values']['postal_code'] != ''){
			$address .= ',+'.$form_state['values']['postal_code'];
		}
		//The getGeoByAddress function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getGeoByAddress($request, false);
		//Output HTML is Determined
		$output = 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=geo_by_address&value='.$address.'">Export URL</a>';
		//drupal_set_message is called
		drupal_set_message(t($output));
	}

	/**
	*The export function processes the data requests.  If it is called via a direct code callup, the data will be returned as a PHP associative array.  If it is called by a post or get as a REST API service (including JavaScript via AJAX), it will print out the data in JSON format.  The user may customize output formats as desired.
	*@param array $_REQUEST or $request
	*@return the requested information is in either PHP associative array or JSON, depending on how the function call is made.
	*/
	public function export($request = ''){
		//Initial values are false
		$controls = array(
			'is_json'=>false,
			'print_out'=>false,
		);
		//If a get or post is detected, the data is transferred over to the $request array.
		if(isset($_REQUEST)){
			$request = $_REQUEST;
			$controls['is_json'] = true;
			$controls['print_out'] = true;
		}
		//The user may modify the output formats as needed to suit their needs.
		$strTypes = array(
			'true'=>true,
			'false'=>false
		);
		foreach($controls as $c=>$cc){
			if(isset($request[$c])){
				if(in_array($request[$c], $strTypes)){
					$controls[$c] = $strTypes[$c];
				}
			}
		}
		if($controls['print_out']){
			//The access control headers are modified to allow cross-domain requests.  This can be adjusted as needed to provide a tighter cross domain control. 
			header("Access-Control-Allow-Origin: *");
			header('Access-Control-Allow-Methods', 'GET POST');
		}
		$obj = '';
		//Switch statement performed for type
		switch($request['type']){
			//DMA infromation is returned
			case 'dma':
				if(!isset($request['value'])){
					$obj = $this->error($controls['is_json']);
				}
				else{
					$obj = $this->getDma($request, $controls['is_json']);
				}
				break;
			//Location by postal code data is returned.
			case 'postal_code':
				if(!isset($request['value'])){
					$obj = $this->error($controls['is_json']);
				}
				else{
					$obj = $this->getPostalCode($request, $controls['is_json']);
				}
				break;
			//Geolocation by address data is returned.
			case 'geo_by_address':
				if(!isset($request['value'])){
					$obj = $this->error($controls['is_json']);
				}
				else{
					$obj = $this->getGeoByAddress($request, $controls['is_json']);
				}
				break;
			//Address by geolocation data is returned.
			case 'address_by_geo':
				if(!isset($request['value'])){
					$obj =  $this->error($controls['is_json']);
				}
				else{
					$obj = $this->getAddressByGeo($request, $controls['is_json']);
				}
				break;
			//City-based information by geolocation is returned.
			case 'city_postal_by_geo':
				if(!isset($request['value'])){
					$obj = $this->error($controls['is_json']);
				}
				else{
					$obj = $this->getCityPostalByGeo($request, $controls['is_json']);
				}
				break;
			//Weather information data is returned
			case 'weather':
				$obj = $this->getWeather($request, $controls['is_json']);
				break;
			//Location by IP address data is presumed to be the default.
			default:
				$obj = $this->getIp($request, $controls['is_json']);
				break;
		}
		db_close();
		//The conditionals for a direct print out of the information
		if($controls['print_out']){
			//Option for printing out the JSON array.
			if($controls['is_json']){
				echo $obj;
			}
			//Prints out the PHP associative array.
			else{
				echo '<pre>';
				print_r($obj);
				echo '</pre>';
			}
		}
		//Returns the information if called up by internal code.
		else{
			return $obj;
		}
	}

	/**
	*The ip_form function returns the Drupal-based form settings for the IP Address lookup form
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function ip_form($form, &$form_state){
		$form['description'] = array(
			'#type'=>'markup',
			'#title'=>'Instructions',
			'#markup'=>'The field may be left blank to resolve your current IP Address of '.$this->ipFind(),
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

	/**
	*The ip_form_submit function processes the IP Address lookup and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function ip_form_submit($form, &$form_state){
		//The IP address will be the user's address by default, unless the ip_address form value is filled in.
		$ipAddress = $this->ipFind();
		if($form_state['values']['ip_address'] != ''){
			$ipAddress = $form_state['values']['ip_address'];
		}
		//The getIp function is called up, with a false value set to return the results as a PHP array.
		$ip = array(
			'value'=>$ipAddress
		);
		$results = $this->getIp($ip, false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=ip_address&value='.$ipAddress.'" target="_blank">Export URL</a>';
		//$output .= '<br />ip is ~'.$form_state['values']['ip_address'].'~.';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The latLon1_form function returns the Drupal-based form settings for the City lookup by lat/lon.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function latlon1_form($form, &$form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The latLon1_form_validate function ensures the latitude and longitude code value are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if either of the two values are blank
	*/
	public function latLon1_form_validate($form, &$form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	/**
	*The latLon1_form_submit function processes the city lookup by lat/lon and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function latLon1_form_submit($form, &$form_state){
		//Combines the lat and lon values for the standard comma-separated value.
		$latLon = $form_state['values']['lat'].','.$form_state['values']['lon'];
		//The getCityPostalByGeo function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getCityPostalByGeo(array('value'=>$latLon), false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=city_postal_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The latLon1_form function returns the Drupal-based form settings for the full address lookup by lat/lon.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function latLon2_form($form, &$form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The latLon2_form_validate function ensures the latitude and longitude code value are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if either of the two values are blank
	*/
	public function latLon2_form_validate($form, &$form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	/**
	*The latLon2_form_submit function processes the address lookup by lat/lon and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function latLon2_form_submit($form, &$form_state){
		//Combines the lat and lon values for the standard comma-separated value.
		$request['value'] = $form_state['values']['lat'].','.$form_state['values']['lon'];
		//The getAddressByGeo function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getAddressByGeo($request, false);
		//Output HTML is determined.
		$output = $results['results']['address'].'<br />'.$results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=address_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The menu function containes all of the UI menu items for the user to interact with.
	*@return The $items array is returned for Drupal to properly process each menu item and what to do for each page callup.
	*/
	public function menu(){
		//The IP Address lookup menu item
		$items['geolocation/ip_address'] = array(
			'title'=>'IP Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_ip_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The Postal Code lookup menu item
		$items['geolocation/postal_code'] = array(
			'title'=>'Postal Code Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_pc_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The City, State, Zip lookup by lat/lon
		$items['geolocation/lat_lon_city'] = array(
			'title'=>'City Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_latlon1_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The comprehensive address lookup by lat/lon
		$items['geolocation/lat_lon_address'] = array(
			'title'=>'Reverse Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_latlon2_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The lat/lon lookup by address
		$items['geolocation/address'] = array(
			'title'=>'Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_address_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The export page for API callups.
		$items['geolocation/export'] = array(
			'title'=>'Geolocation Export',
			'type'=>MENU_CALLBACK,
			'page callback'=>'geolocation_basic_export',
			'access arguments'=>array('access content'),
		);
		//The weather page
		$items['geolocation/weather'] = array(
			'title'=>'Geolocation Weather',
			'type'=>MENU_CALLBACK,
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_weather_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
		);
		return t($items);
	}

	/**
	*The permission function returns to Drupal the permission settings.
	*@return The permission settings array.
	*/
	public function permission(){
		return array(
	    	'access geolocation content' => array(
	     		'title' => t('Access Geolocation Content'),
	    	)
	  	);
	}

	/**
	*The pcForm function returns the Drupal-based form settings for the Postal Code lookup form
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function pc_form($form, &$form_state){
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

	/**
	*The pc_form_validate function ensures the postal code value is filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if the postal code value is blank
	*/
	public function pc_form_validate($form, &$form_state){
		if (($form_state['values']['postal_code'] == '')){
			return form_set_error('postal_code', t('A Postal Code is required.'));
		}
	}

	/**
	*The pc_form_submit function processes the Postal Code lookup and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function pc_form_submit($form, &$form_state){
		//The getPostalCode function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getPostalCode($form_state['values']['postal_code'], false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=postal_code&value='.$form_state['values']['postal_code'].'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The weatherForm function returns the Drupal-based form settings for the weather lookup, along with lookup instructions.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function weather_form($form, &$form_state){
		$form['description'] = array(
			'#type'=>'markup',
			'#title'=>'Instructions',
			'#markup'=>'Enter either Latitude/Longitude coordinates, a postal code, or an IP address for real time weather information.  Latitude/Longitude will take precidence over postal code, and postal code will take precidence over an IP address.  If no fields are entered, the system will pull information based on your current IP address.  Default weather results are for 1 full day, unless a value is otherwise indicated in the Date Range field.',
		);
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lng'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
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
		//The date range (end) field can utilize the PHP technique for determining the end time, i.e. +2 days
		$form['end'] = array(
			'#type'=>'textfield',
			'#title'=>'Date Range',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The weatherFormSubmit function processes the weather lookup and returns the results to the user.
	*/
	public function weather_form_submit($form, &$form_state){
		$request = array();
		$output = '';
		//Inserts the end value into the $request array if it is set.
		if($form_state['values']['end'] != ''){
			$request['end'] = $form_state['values']['end'];
		}
		//If both latitude and longitude values are filled out, they take precidence and the weather information is determined by these values.
		if($form_state['values']['lat'] != '' && $form_state['values']['lng'] != ''){
			$request['subtype'] = 'latlng';
			$request['value'] = $form_state['values']['lat'].','.$form_state['values']['lng'];
		}
		//If lat/lon were not filled out, but the postal code is, the weather is looked up by postal code as the location.
		elseif($form_state['values']['postal_code'] != ''){
			$request['subtype'] = 'postal_code';
			$request['value'] = $form_state['values']['postal_code'];
		}
		//The weather will be looked up by using the user's IP address as the location
		else{
			$request['subtype'] = 'ip_address';
			//The system will take the IP address filled out by the user, or utilize the user's current IP if it isn't filled out.
			if($form_state['values']['ip_address'] != ''){
				$request['value'] = $form_state['values']['ip_address'];
			}
			else{
				$request['value'] = $this->ipFind();
			}
		}
		//Export URL is determined
		$exportUrl = $GLOBALS['base_url'].'/geolocation/export?type=weather&subtype='.$request['subtype'].'&value='.$request['value'];
		if($form_state['values']['end'] != ''){
			$exportUrl .= '&end=+'.urlencode($form_state['values']['end']);
		}
		//The getWeather function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getWeather($request, false);
		//If the results do include weather information...
		if($results['status'] = 'success'){
			//The human-readable location, NWS source XML, and comprehensive NWS weather page are added to the $output variable.
			$output .= '<h2>Weather Information For '.$results['results']['city'].', '.$results['results']['state_region'].'</h2>';
			$output .= '<p><a href="'.$exportUrl.'" target="_blank">Export URL</a><br />';
			$output.='<a href="'.$results['results']['nws_xml'].'" target="_blank">NWS XML Source</a><br />';
			$output.= '<a href="'.$results['results']['nws_page'].'" target="_blank">NWS Page</a></p>';
			//Weather for current day, which will show the overall weather icon, current temp, high temp, preciptation, cloud cover, 12 hour precipitation probability, humidity, and wind data.
			$output .= '<div><h3>Today</h3>';
			$output .= '<img src="'.$results['results']['data']['icon'][0]['value'].'" /><br />';
			$output .= 'Temperature: '.$results['results']['data']['hourly_temp'][0]['value'].' - '.$results['results']['data']['maximum_temp'][0]['value'].'<br />';
			$output .= 'Precipitation: '.$results['results']['data']['precipitation'][0]['value'].'<br />';
			$output .= 'Cloud Cover: '.$results['results']['data']['cloud_cover'][0]['value'].'<br />';
			$output .= '12 Hour Probability of Precipitation: '.$results['results']['data']['12_hour_precip_prob'][0]['value'].'<br />';
			$output .= 'Humidity: '.$results['results']['data']['humidity'][0]['value'].'<br />';
			$output .= 'Winds out of the '.$results['results']['data']['wind_dir'][0]['value'].' at '.$results['results']['data']['wind_speed'][0]['value'].'</div>';
			$days = count($results['results']['data']['maximum_temp']);
			//If the results have multiple days, the system will loop through and add in the name of the day, weather icon, minimum temp, and maximum temp.
			if($days > 1){
				$output .= '<div id="weather_days">';
				for($i=1; $i<$days; $i++){
					//Ensures the time value can be properly read.
					$wDate = explode('T', $results['results']['data']['maximum_temp'][$i]['start_valid_time']);
					//A regular expression for the date is formulated because of the wide variations in the time coverage.
					$wdc = '/'.$wDate[0].'T10:00:00/i';
					//Only the icon is needed for the time being, but the other values are there just in case.
					$wVals = array(
						'precipitation'=>'',
						'cloud_cover'=>'',
						'icon'=>'',
					);
					//Each of the values are looped through to extract the proper information.
					foreach($wVals as $w=>$ww){
						for($t=0; $t<count($results['results']['data'][$w]); $t++){
							if(preg_match($wdc, $results['results']['data'][$w][$t]['start_valid_time'])){
								$wVals[$w] = $results['results']['data'][$w][$t]['value'];
								break;
							}
						}
					}
					//The date-specific weather information is added to the output.
					$output .= '<div><h3>'.date('l', strtotime($wDate[0])).'</h3>';
					$output .= '<img src="'.$wVals['icon'].'" /><br />';
					$output .= $results['results']['data']['minimum_temp'][$i-1]['value'].' - '.$results['results']['data']['maximum_temp'][$i]['value'].'</div>';
				}
				$output .= '</div>';
			}
		}
		//If no results were returned, the system grabs the results message for output.
		else{
			$output .= $results['msg'];
		}
		//drupal_set_message is run
		drupal_set_message(t($output));
	}
}