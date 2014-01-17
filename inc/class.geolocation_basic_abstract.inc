<?php
/**
*This script houses the Geolocation Basic Module abstract class and contains the module's data abstraction logic.
*@link http://www.jeremyrperry.com
*@version 3.0
*@author Jeremy Perry jperry@phluant.com, jeremyrperry@gmail.com
*@package Phluant Mobile Drupal 7 Geolocation Module
*@todo Created on 01-15-2014 to provide a cleaner separation of presentation and data abstraction logic.  Version also provides mongodb support for appropriate database tables.  Proper mongodb setup and installation of the Drupal mongodb module is required for mongodb features to work.
*
*/

abstract class geolocation_basic_abstract{
	protected $mapquest_api;
	protected $mdb;
	//Adjust this boolean to use mongodb throughout the class if desired.  This will not affect the IP lookup if binary is selected.
	public $useMongo = false;
	//Adjust this variable to change the desired IP lookup source.
	public $ipSource = 'binary';

	/**
	*The destruct function ensures the default database is set so there are no potential errors.
	*@return see above.
	*/
	function __destruct(){
		db_set_active('default');
	}

	/**
	*The apiLimitCheck function checks the inputted API object against the maximum API limits and returns the allowed number of API callups remaining
	*@param object $obj
	*@return The remaining value and database query are outputted to array fromat.
	*/
	protected function apiLimitCheck($obj){
		//Query values are declared.
		$timestamp = strtotime('-'.$obj->limit_time_unit);
		$queryArr = array(
			':api_val_id'=>$obj->id,
			':timestamp'=>$timestamp,
		);
		//Query is ran, and values are outputted.
		$query = "select sum(insert_count) as used from geolocation_api_track where api_val_id = '".$obj->id."' and timestamp >= '".$timestamp."'";
		$q = db_query($query);
		$r = $q->fetchAssoc();
		$output['value'] = $obj->limit_qty - $r['used'];
		$output['query'] = $query;
		return $output;
	}

	/**
	*The degreesToDirection function is able to take a degree and convert it into the corresponding geographic direction.  It works in conjunction with the getWeather function.
	*@param string $degree
	*@return The degree number is converted into the corresponding geographical direction.
	*/
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
			return 'NNW';
		}
	}

	/**
	*The error function is available throughout the object to return an error notifying the user of an invalid request.
	*@param boolean $isJson, defaults to true.
	*@return The error message data in either JSON or PHP associative array format.
	*/
	protected function error($isJson = true){
		$output['status'] = 'error';
		$output['msg'] = 'invalid request';
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getApi function allows for an api value to be called from the database .
	*@param string $name
	*@return The database result object
	*/
	protected function getApi($name){
		db_set_active('default');
		$q = db_query("select * from geolocation_api_values where name = :name", array(':name'=>$name));
		return $q->fetchObject();
	}

	/**
	*The getAddressByGeo function is set up to make an API call the Google Maps Geocoder to get the full address of the inputted geocoordinates.  It outputs a much more simplified version of the original Google data.  Because of the query limitations imposed by Google, this function is used sparingly.
	*@param array $request
	*@param boolean $isJson default is true.
	*@return Geoolocation information in either JSON or PHP associative array format.
	*@todo This function was modified on 1-15-2014 to reflect input changes from a string to an array.
	*/
	protected function getAddressByGeo($request, $isJson = true){
		//The geocoordinates are presumed to be in the correct comma delimited format, so the API query is ran immediately.
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?latlng='.$request['value'].'&sensor=false';
		//The information returned is decoded from JSON for PHP to work with the data easiery.
		$json = json_decode(file_get_contents($url), true);
		//The address is obtained by exploding the formatted address field out into an array.
		$address = explode(',', $json['results'][0]['formatted_address']);
		$results['address'] = trim($address[0]);
		$results['city'] = trim($address[1]);
		//The state and postal code have a slightly modified method of retreival.
		$stateZip = explode(' ', trim($address[2]));
		$results['state_region'] = $stateZip[0];
		$results['postal_code'] = $stateZip[1];
		$results['country'] = trim($address[3]);
		$output['status'] = 'success';
		$output['results'] = $results;
		//Pulls DMA information if it is requested.
		if($request['dma_info']){
			$dmaArr = array(
				'subtype'=>'geo',
				'value'=>$geo,
			);
			$dma = $this->getDma($dmaArr, false);
			if($dma['status'] == 'success'){
				foreach($dma['results'] as $d){
					$output['results'][$d] = $dma['results'][$d];
				}
			}
		}
		//Converts to JSON format unless otherwise specified, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getCityPostalByGeo function provides a lookup of the nearest city, state, and zip based in inputted geocoordinates.
	*@param array $request
	*@param boolean $isJson default is true.
	*@return Geolocation information in either JSON or PHP associative array format.
	*@todo This function was modified on 1-15-2014 to provide mongodb support.
	*/
	protected function getCityPostalByGeo($request, $isJson = true){
		//The latitude and longitude is determined by a comma separation in $request['geo'].
		$coords = explode(',', $request['value']);
		$lat = $coords[0];
		$lng = $coords[1];
		$output = array();
		$rowCount = 0;
		$dist = 100;
		if(isset($request['dist']) && is_numeric($request['dist'])){
			$dist = $request['dist'];
		}
		//Mongodb sequence
		if($this->useMongo){
			$mGeo = $this->mongoDb('geolocation');
			/*
			//This is for running a query based on a 2d index.  Left as a reference.
			$radius = 10/3959;
			$find = array(
				'loc'=>array(
					'$within'=>array(
						'$centerSphere'=>array(
							array(floatval($lng), floatval($lat)), $radius
						),
					),
				),
			);
            */
			//Array for the geolocation infromation is formualated.  The $lat and $lng values must be cast as f
			$find = array(
				'loc'=>array(
					'$near'=>array(
						'$geometry'=>array(
							'type'=>'Point',
							'coordinates'=>array(floatval($lng), floatval($lat))
						),
						'$maxDistance'=>$dist
					),
				),
            );
			$q = $mGeo->geolocation_cities->find($find)->limit(1);
			$rowCount = $q->count();
			if($rowCount  > 0){
				$output['status'] = 'success';
				foreach($q as $r){
					$output['results'] = $r;
				}
				$output['msg'] = 'by mongo';
			}
		}
		//RDBMS sequence
		else{
			$lng1 = $lng-$dist/abs(cos(deg2rad($lat))*69.09);
			$lng2 = $lng+$dist/abs(cos(deg2rad($lat))*69.09);
			$lat1 = $lat-($dist/69.09);
			$lat2 = $lat+($dist/69.09);
			$insertVals = array(
				':lat0'=>$lat,
				':lng0'=>$lng,
				':lat1'=>$lat1,
				':lat2'=>$lat2,
				':lng1'=>$lng1,
				':lng2'=>$lng2,
			);
			$query = "select gc.*, round(degrees(acos(sin(radians(:lat0))*sin(radians(gc.lat))+cos(radians(:lat0))*cos(radians(gc.lat))*cos(radians(:lng0 - gc.lng)))) *69.09) as distance from geolocation_cities as gc where gc.postal_code != '' and gc.lng between :lng1 and :lng2 and gc.lat between :lat1 and :lat2 order by distance asc limit 1";
			$q = db_query($query, $insertVals);
			//Query results are good
			if($q){
				$rowCount = $q->rowCount();
				if($rowCount >0){
					$r = $q->fetchAssoc();
					$output['status'] = 'success';
					$output['results'] = $r;
				}
			}
		}
		if($rowCount == 0){
			$output = $this->getAddressByGeo($request, false);
		}
		//Output is converted to JSON unless the boolean was set to false, database set to default, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getDma function pulls up the closest possible DMA based on the inputted geolocation value.  Because of variations in DMA boundaries, this function may have a noticeable margin of error.
	*@param array $request
	*@param boolean $isJson
	*@return DMA information
	*@todo This function was modified on 1-15-2014 to provide mongodb support, utilize the dma code as the default, and to reflect input changes from a string to an array.
	*/
	protected function getDma($request, $isJson = true){
		//Output array is set
		$output = array();
		//The mongodb database object is created if mongo is being used.
		if($this->useMongo){
			$mGeo = $this->mongoDb('geolocation');
		}
		switch($request['subtype']){
			//Uses geocoordinates to determine dma information
			case 'geo':
				$geo = explode(',', $request['value']);
				$dist = 1000;
				$lat = $geo[0];
				$lng = $geo[1];
				$runAlt = false;
				//Mongodb sequence
				if($this->useMongo){
					//Geolocation find array is created.
					$find = array(
						'loc'=>array(
							'$near'=>array(
								'$geometry'=>array(
									'type'=>'Point',
									'coordinates'=>array(floatval($lng), floatval($lat))
								),
								'$maxDistance'=>$dist
							),
						),
		            );
		            //Mongo query is run, and checked for a result
					$q = $mGeo->geolocation_cities->find($find)->limit(1);
					if($q->count() > 0){
						$dma = '';
						//Dma code is retreived
						foreach($q as $r){
							$dma = $r['dma_code'];
						}
						$find2 = array(
							'dma_code'=>array($dma)
						);
						//Secondary query is run to get DMA info
						$q2 = $mGeo->geolocation_dma->find($find2);
						if($q2->count() > 0){
							$output['status'] = 'success';
							foreach($q2 as $r2){
								$output['results'] = $r2;
							}
						}
					}
					//If no results were found from the primary search method, the system will attempt to find the closest DMA boundary directly from the dma table.  This method can be inaccurate due to variations in DMA boundaries.
					else{
			            $qAlt = $mGeo->geolocation_dma->find($find)->limit(1);
			            if($qAlt->count() > 0){
			            	$output['status'] = 'success';
			            	foreach($qAlt as $rAlt){
			            		$output['results'] = $rAlt;
			            	}
			            }
			            else{
			            	$output['status'] = 'error';
			            	$output['msg'] = 'query failure';
			            }
					}
				}
				//RDBMS sequence
				else{
					$lng1 = $lng-$dist/abs(cos(deg2rad($lat))*69.09);
					$lng2 = $lng+$dist/abs(cos(deg2rad($lat))*69.09);
					$lat1 = $lat-($dist/69.09);
					$lat2 = $lat+($dist/69.09);
					$insertVals = array(
						':lat0'=>$lat,
						':lng0'=>$lng,
						':lat1'=>$lat1,
						':lat2'=>$lat2,
						':lng1'=>$lng1,
						':lng2'=>$lng2,
					);
					//The next four lines draw up the latitude/longitude permiter to search in, and modified for PHP.  It uses the values from $lat, $lng, and $dist to perform the calculations.  It is based on that 1 degree of latitude is equivelant to 69.09 miles and 1 degree of longitude is equivelant to the cosign of the latitude times 69.09.  For more detailed information, see slides 10 and 11 of the GeoSpacial MySQL Search whitepaper at http://www.scribd.com/doc/2569355/Geo-Distance-Search-with-MySQL.
					$query = "select gd.region_name, gd.adperc, gd.tvperc, gd.cableperc, round(degrees(acos(sin(radians(:lat0))*sin(radians(gc.lat))+cos(radians(:lat0))*cos(radians(gc.lat))*cos(radians(:lng0 - gc.lng)))) *69.09) as distance from geolocation_cities as gc left join geolocation_dma as gd on gc.dma_code = gd.dma_code where gc.lng between :lng1 and :lng2 and gc.lat between :lat1 and :lat2 order by distance limit 1";
					$queryAlt = 'select *, round(degrees(acos(sin(radians(:lat0))*sin(radians(lat))+cos(radians(:lat0))*cos(radians(lat))*cos(radians(:lng0 - lng)))) *69.09) as distance from geolocation_dma where lng between :lng1 and :lng2 and lat between :lat1 and :lat2 order by distance limit 1';
					//Query is set and run
					$q = db_query($query, $insertVals);
					//Will run the secondary query if there was a query problem
					if(!$q){
						$runAlt = true;
					}
					
					else{
						//If no data was returned, the secondary query will be run
						if($q->rowCount() < 1){
							$runAlt = true;
						}
						//If query is successful, results are outputted.
						else{
							$output['status'] = 'success';
							$output['results'] = $q->fetchAssoc();
						}
					}
					//Conditional for the secondary query to be run
					if($runAlt){
						$alt = db_query($queryAlt, $insertVals);
						if(!$alt){
							$output['status'] = 'error';
							$output['error_msg'] = 'query failure';
						}
						else{
							$output['status'] = 'success';
							$output['results'] = $alt->fetchAssoc();
						}
					}
				}
				break;
			//The default presumes the value is the dma code is present
			default:
				//Mongodb sequence
				if($this->useMongo){
					//Query is set and run.
					$find = array(
						'dma_code'=>floatval($request['value'])
					);
					$q = $mGeo->geolocation_dma->find($find);
					//Successful query.
					if($q->count() > 0){
						$output['status'] = 'success';
						foreach($q as $r){
							$output['results'] = $r;
						}
					}
					//Error message outputs if no results are present
					else{
						$output['status'] = 'error';
						$output['error_msg'] = 'query failure';
					}
				}
				//RDBMS sequence
				else{
					$query = "select * from geolocation_dma where dma_code = :dma_code";
					$insertVals = array(
						':dma_code'=>$request['value'],
					);
					//Query is set and run
					$q = db_query($query, $insertVals);
					//Outputs an error message if there was a problem with the query
					if(!$q){
						$output['status'] = 'error';
						$output['error_msg'] = 'query failure';
					}
					//If query is successful, results are outputted.
					else{
						$output['status'] = 'success';
						$output['results'] = $q->fetchAssoc();
					}
				}
				break;
		}
		$keep = array('region_name', 'adperc', 'tvperc', 'cableperc');
		foreach($output['results'] as $r=>$rr){
			if(!in_array($r, $keep)){
				unset($output['results'][$r]);
			}
		}
		//Output is converted into JSON by default, unless otherwise specified.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getGeoByAddress function does a latitude/longitude lookup based on the full address through the Google Maps Geocoder API.  Because of the query limitations imposed by Google, this function is used sparingly and only when exact street address based coordinates are needed.
	*@param array $request
	*@param boolean $isJson default is true
	*@return Latitude/longitude information in either JSON or PHP associative array format.
	*@todo This function was modified on 1-15-2014 to reflect changes in the input from string to array.
	*/
	protected function getGeoByAddress($request, $isJson = true){
		//Address is URL encoded to ensure the API query can be run properly.
		$address = urlencode($request['value']);
		//API query is run
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?address='.$address.'&sensor=false';
		//Data is decoded from JSON so PHP can work with it easier.
		$json = json_decode(file_get_contents($url), true);
		$address = explode(',', $json['results'][0]['formatted_address']);
		$results['city'] = trim($address[1]);
		//The state and postal code have a slightly modified method of retreival.
		$stateZip = explode(' ', trim($address[2]));
		$results['state_region'] = $stateZip[0];
		$results['postal_code'] = $stateZip[1];
		$results['country'] = trim($address[3]);
		$results['lat'] = $json['results'][0]['geometry']['location']['lat'];
		$results['lng'] = $json['results'][0]['geometry']['location']['lng'];
		$output['status'] = 'success';
		$output['results'] = $results;
		//Pulls dma information if requested.
		if($request['dma_info']){
			$dmaArr = array(
				'subtype'=>'geo',
				'value'=>$results['lat'].','.$results['lng'],
			);
			$dma = $this->getDma($dmaArr, false);
			if($dma['status'] == 'success'){
				foreach($dma['results'] as $d=>$dd){
					$output['results'][$d] = $dma['results'][$d];
				}
			}
		}
		//Converts to JSON format unless otherwise specified, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getGeoByAddressMq function is the MapQuest version of getGeoByAddress (lat/lon lookup by full address).  Because MaqQuest has more generous API lookups with their community edition, this function is designed for bulk lookups (i.e. location information importing).
	*@param array $address
	*@param boolean $isJson defaults to true.
	*@return The full street address geolocation information in either JSON or PHP associative array format.
	*@todo This function was modified on 1-15-2014 to reflect changes in the input from string to array.
	*/
	protected function getGeoByAddressMq($request, $isJson = true){
		$address = $request['value'];
		if(!defined($this->mapquest_api)){
			$this->mapquest_api = $this->getApi('mapquest');
		}
		$results = array();
		//Counts out the number of addresses.
		$count = count($address);
		//Calls up the apiLimitCheck function to see how many API calls are remaining
		$limit = $this->apiLimitCheck($this->mapquest_api);
		//The system will resolve as many addresses as possible, but will omit any values exceeding the API limitation.
		if($limit['value'] < $count){
			$output['msg'] = 'Notice: '.$limit.' of the '.$count.' records without lat/lon values were not updated due to API limitations.';
			$count = $limit['value'];
		}
		//Sends a notice to the user when less than 1000 API calls remain.
		elseif($limit['value'] > $count){
			$remainder = $limit['value'] - $count;
			if($remainder > 1000){
				$output['msg'] = 'Notice: there are '.$remainder.' API calls remaining for this API\'s limitation settings.  Contact an admin for more information.';
			}
		}
		$results = array();
		//MapQuest can run batches of 100 addresses at a time, so the batch number is determined by that fact.
		$batchNumber = ceil($count/100);
		//Loops though the number of batch lookups to be performed
		for($i=0; $i<$batchNumber; $i++){
			$start = $i*100;
			//Determines the number of lookups to be performed in the current batch, particularly because the last batch will very unlikely have 100 lookups.
			if($i+1 == $batchNumber){
				$end = $count;
			}
			else{
				$end = $start+100;
			}
			//The addresses in the batch are added to the location variable.
			$location = '';
			for($m=$start; $m<$end; $m++){
				$location .= '&location='.urlencode($address[$m]['address']);
			}
			//The URL is formulated. It utilizes an API key stored in the database.
			$url = 'http://www.mapquestapi.com/geocoding/v1/batch?key='.$this->mapquest_api->api_key.'&callback=renderBatch'.$location;
			//The content retreived is processed as necessary for PHP to efficiently read the incomming JSON
			$json = file_get_contents($url);
			$json = str_replace('renderBatch(', '', $json);
			$json = substr($json, 0, -2);
			$arr = json_decode($json, true);
			$rCount = count($arr['results']);
			//The results are looped through to populate the lat/lon information.
			for($r=0; $r<$rCount; $r++){
				$indexAt = $start+$r;
				$mqResults['index'] = $address[$indexAt]['index'];
				$mqResults['lat'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lat'];
				$mqResults['lng'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lng'];
				$results[] = $mqResults;
			}
		}
		//The database is updated with the number of API lookups performed
		$insert = db_insert('geolocation_api_track')
			->fields(array(
				'api_val_id'=>$this->mapquest_api->id,
				'insert_count'=>$count,
				'timestamp'=>REQUEST_TIME
			))
			->execute();
		///Results finalized, formatted as JSON unless specified otherwise, and returned.
		$output['status'] = 'success';
		$output['results'] = $results;
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getIp function is able to resolve a user's IP address to their local area.  By default, it calls out to freegeoip.net, but this can be overwritten by appropriate settings.  A freegeoip.net callout will still be utilized as a backup.
	*@param string $ipAddress, boolean $json defaults as true.
	*@return The user's location information in either JSON or PHP associative array format.
	*@todo This function was extensively modified on 1-15-2014 to reflect changes in the input from string to array, provide support for Maxmind's binary database, provide support for mongodb, to permit for use of freegeoip as a default/backup, and provides the DMA specific info only if requested.
	*/
	protected function getIp($request, $isJson = true){
		$output = array();
		$ipAddress = $this->ipFind();
		if(isset($request['value']) && $request['value'] != ''){
			$ipAddress = $request['value'];
		}
		//Settings are declared.  They can be overridden as needed for desired default operations.
		$settings = array(
			'source'=>$this->ipSource,
			'ip_correct'=>true,
			'dma_info'=>false,
			'run_update'=>true
		);
		//Each setting above can be overridden with the proper $request value.
		foreach($settings as $s=>$ss){
			if(isset($request[$s])){
				$settings[$s] = $request[$s];
			}
		}
		//Mongo is set if needed
		if($this->useMongo && $settings['source'] != 'binary'){
			$settings['source'] = 'mongo';
		}
		$noLocation = false;
		$runUpdate = false;
		switch($settings['source']){
			//The mondodb sequence.  Will failover to freegeoip if no results are found.
			case 'mongo':
				//A second boolean declaration is used in case the default setting was overridden in the corresponding $request value.
				$this->useMongo = true;
				//Database instance is declared, ipv4 converted, and query is run.
				$mGeo = $this->mongoDb('geolocation');
				$ipv4 = $this->getIpv4($ipAddress);
				$find = array(
					'start_ip_num'=>array(
						'$lte'=>$ipv4,
					),
					'end_ip_num'=>array(
						'$gte'=>$ipv4,
					)
				);
				$q = $mGeo->geolocation_ip_locations->find($find)->limit(1);
				$locId = '';
				//If results are found, the secondary query is run.
				if($q->count() > 0){
					foreach($q as $r){
						$locId = $r['loc_id'];
					}
					//Secondary query is run.  Breaks if results are found.
					$rq = $mGeo->geolocation_cities->find(array('id'=>$locId));
					if($rq->count > 0){
						foreach($rq as $rr){
							$output['results'] = $rr;
						}
						$output['status'] = 'success';
						$output['msg'] = 'from mongo';
						break;
					}
				}
			//The RDBMS sequence.  Fails over to freegeoip if no results are found.
			case 'rdbms':
				//Query is run against the database tables.
				$q = db_query("select c.* from geolocation_ip_locations as l left join geolocation_cities as c on l.loc_id = c.id where INET_ATON(:ipa) between l.start_ip_num and l.end_ip_num limit 1", array(':ipa'=>$ipAddress));
				//No results returned
				if(!$q){
					$output['msg'] = 'Query failure.  Using Freegeoip.';
					$noLocation = true;
				}
				//Results were returned.
				else{
					$r = $q->fetchAssoc();
					$locId = $r['id'];
					//Checks to ensure the location ID is present.  Lack of one means empty results, and a failover to freegeoip.
					if($locId == '' || is_null($locId)){
						$noLocation = true;
						$output['msg'] = 'Can\'t resolve by database.  Using Freegeoip.';
					}
					else{
						$output['results'] = $r;
						$output['source'] = 'rdbms';
						$runUpdate = true;
					}
				}
				if(!$noLocation){
					break;
				}
			//The Maxmind binary db sequence.  Using this procedure is strongly recommended.
			case 'binary':
				//Calls up the binary load script.  Please refer to the script for more information.
				require_once str_replace('/inc', '', dirname(__FILE__)).'/geolocation_basic_binLoad.php';
				//The binary load script is set to load the geolite version by default.  The full version may also be used, provided one has the proper licencing requrements.
				$binarySrc = 'geoip';
				if(isset($request['binary_src'])){
					$binarySrc = $request['binary_src'];
				}
				//Binary db function callup, and ensured variable is a proper object.
				$reader = getBinaryDb($binarySrc);
				if(!is_object($reader)){
					$noLocation = true;
					$output['msg'] = $binarySrc.' is not a valid database.  Using Fregeoip.';
				}
				else{
					//The IP address record is produced.
					$record = $reader->city($ipAddress);
					$output['results'] = array(
						'country'=>$record->country->isoCode,
						'state_region'=>$record->mostSpecificSubdivision->isoCode,
						'city'=>$record->city->name,
						'postal_code'=>$record->postal->code,
						'lat'=>$record->location->latitude,
						'lng'=>$record->location->longitude,
						'dma_code'=>$record->location->metroCode
					);
					//Ensures the binary db results contain the proper data, and fails over to freegeoip if any are missing.
					foreach($output['results'] as $r=>$rr){
						if ($rr == '' || is_null($rr)){
							$noLocation = true;
							$output['msg'] = 'One or more binary db attributes were missing.  Using Freegeoip.';
						}
					}
				}
				if(!$noLocation){
					$output['source'] = 'binary db';
					break;
				}
			//The default and failover from any method above is to use freegeoip.  This function alone will work out of the box with no extra configuration, but freegeoip does impose query limits.  For sites where high volumes of IP lookups will be used, a more advanced primary lookup method is strongly recommended.
			default:
				$results = json_decode(file_get_contents("http://freegeoip.net/json/".$ipAddress),true);
				$output['results']['country'] = $results['country_name'];
				$output['results']['state_region'] = $results['region_name'];
				$output['results']['city'] = $results['city'];
				$output['results']['postal_code'] = $results['zipcode'];
				$output['results']['lat'] = $results['latitude'];
				$output['results']['lng'] = $results['longitude'];
				$output['results']['metro_code'] = $results['metro_code'];
				$output['results']['area_code'] = $results['areacode'];
				$output['source'] = 'freegeoip';
		}
		$dataGood = true;
		$checkFor = array('lat','lng');
		foreach($checkFor as $c){
			if($output['results'][$c] == '' || is_null($output['results'][$c])){
				$dataGood = false;
				break;
			}
		}
		if($dataGood){
			$output['status'] = 'success';
			//Correcting the IP information is run by default, unless it is disabled for performance.
			if($settings['ip_correct']){
				$output['results'] = $this->ipCorrect($output['results'], $settings['run_update']);
			}
			//The extra DMA information will be retreived if requested.
			if($settings['dma_info']){
				$dmaArr = array(
					'subtype'=>'dma_code',
					'value'=>$output['results']['dma_code'],
				);
				$dma = $this->getDma($dmaArr, false);
				if($dma['status'] == 'success'){
					foreach($dma['results'] as $d){
						$output['results'][$d] = $dma['results'][$d];
					}
				}
			}
		}
		//The lat/lon checks failed, no bueno!
		else{
			$output['status'] = 'error';
			$output['msg'] = 'Unable to resolve IP address';
		}
		//Data will be converted to JSON unless boolean is false.
		if($isJson){
			$output = json_encode($output);
		}
		//Data is returned to the callup source.
		return $output;
	}

	/**
	*The getIpv4 function is able to convert an IP Address into an Ipv4 value.  It is a backup just in case the database query can't perform this task directly.
	*@param string $ip
	*@return the converted Ipv4 value
	*/
	protected function getIpv4($ip){
		$ipArr = explode('.', $ip);
		$output =  (16777216 * $ipArr[0])+(65536 * $ipArr[1]) + (256 * $ipArr[2])+ $ipArr[3];
		return $output;
	}

	/**
	*The getPostalCode function provides a lookup of the City, State, Latitude, and Longitude of an inputted postal code value.
	*@param string $postalCode
	*@param boolean $isJson default is true.
	*@return Geolocation information in either JSON or PHP associative array format.
	*@todo This function was modified on 1-15-2014 to reflect changes in the input from string to array, and to provide mongodb support.
	*/	
	protected function getPostalCode($request, $isJson = true){
		$postalCode = $request['value'];
		$output = array();
		$rowCount = 0;
		//Query is run, depending on database sequence used.  Attempts to pull from the geolocation_cities database table first.
		//Sequence for mongodb.
		if($this->useMongo){
			//Query is run.
			$mGeo = $this->mongoDb('geolocation');
			$q = $mGeo->geolocation_cities->find(array('postal_code'=>$postalCode))->limit(1);
			$rowCount = $rq->count();
			//Successful query.
			if($rowCount > 0){
				$output['status'] = 'success';
				foreach($q as $r){
					$output['results'] = $r;
				}
			}
		}
		//Sequence for RDBMS
		else{
			$q = db_query("select * from geolocation_cities where postal_code = :pc limit 1", array(':pc'=>$postalCode));
			//Successful query
			if($q){
				$rowCount = $q->rowCount();
				if($q->rowCount() > 0){
					$r = $q->fetchAssoc();
					$output['status'] = 'success';
					$output['results'] = $r;
				}
			}
		}
		//If no results could be found, the system will call up the getGeoByAddress function (results from Google's API).
		if($rowCount == 0){
			$output = $this->getGeoByAddress($request, false);
		}
		//Output converted to JSON by default.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getWeather function is able to pull a National Weather Service weather feed based on the user's geocoordinates.  Its main job is to simplify the data received from NWS, and provides a local cache to help with performance.
	*@param array $request
	*@param boolean $isJson, default is true
	*@return The weater information in either JSON or PHP associative array format.
	*@todo Modified on 1-15-2014 to provide mongodb support.
	*/
	protected function getWeather($request, $isJson = true){
		$output = array();
		//NWS is set to pull weather based on geocoordinates.  If a postal code or ip address is detected, the system will call up the respective functions to retreive the geocoordinates.
		switch($request['subtype']){
			case 'postal_code':
				$results = $this->getPostalCode($request, false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
				break;
			case 'geo':
				$geo = explode(',', $request['value']);
				$results = $this->getCityPostalByGeo($request, false);
				$lat = $geo[0];
				$lng = $geo[1];
				break;
			//The default presumes the system will pull based on the IP address.  No subtype is technically required.
			default:
				$results = $this->getIp($request, false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
		}
		//Start and end times are set.  A custom end time can be specified from the user or API end using the PHP conversion technique.
		$start = REQUEST_TIME;
		$endString = '+1 day';
		if(isset($request['end'])){
			$endString = '+'.$request['end'];
		}
		$end = strtotime($endString);
		//Because the NwS feed can be sluggish at times, the system is set up to store forecasts in the local database, and will attempt to check for a stored version less than 3 hours old with the same zipcode and forecast length.  In addition, database caches older than three hours and matching the user's postal code and forecast length are deleted.
		$timestamp = $start-(60*60*3);
		db_set_active('default');
		$rowCount = 0;
		$remove = array('_id', 'id', 'timestamp', 'forecast_length');
		$qArray = array(':postal_code'=>$results['results']['postal_code'],':timestamp'=>$timestamp, ':forecast_length'=>$endString);
		//Mongodb lookup/deletion sequence.
		if($this->useMongo){
			$mGeo = $this->mongoDb('geolocation');
			$find = array(
				'postal_code'=>floatval($results['results']['postal_code']),
				'forecast_length'=>$endString,
				'timestamp'=>array(
					'$gte'=>$timestamp
				),
			);
			$rq = $mGeo->geolocation_weather_cache->find($find)->limit(1);
			$rowCount = $rq->count();
			//If a row was returned from the query, that information will be used.
			if($rowCount > 0){
				foreach($rq as $rr){
					foreach($rr as $k=>$v){
						if(in_array($k, $remove)){
							unset($rr[$k]);
						}
					}
					$output['status'] = 'success';
					$output['results'] = $rr;
					
				}
			}
			$remove = array(
				'postal_code'=>floatval($results['results']['postal_code']),
				'timestamp'=>array('$lt'=>$timestamp),
				'forecast_length'=>$endString
			);
			$mGeo->geolocation_weather_cache->remove($remove, true);
		}
		//RDBMS sequence
		else{
			$query = 'select * from geolocation_weather_cache where postal_code = :postal_code and timestamp >= :timestamp and forecast_length = :forecast_length order by timestamp desc limit 1';
			$q = db_query($query, $qArray);
			$rowCount = $q->rowCount();
			//If a row was returned from the query, that information will be used.
			if($rowCount > 0){
				$r = $q->fetchAssoc();
				foreach($r as $k=>$v){
					if(in_array($k, $remove)){
						unset($r[$k]);
					}
				}
				//Database results are inputted into the output array
				$r['data'] = json_decode($r['data'], true);
				$output['status'] = 'success';
				$output['results'] = $r;
			}
			db_query('delete from geolocation_weather_cache where postal_code = :postal_code and timestamp < :timestamp and forecast_length = :forecast_length', $qArray);
		}
		if($rowCount == 0){
			//The time values are converted to the format needed by the NWS.
			$timeStart = date('Y-m-d', $start).'T'.date('H:i:s', $start);
			$timeEnd = date('Y-m-d', $end).'T'.date('H:i:s', $end);
			//NWS link is determined and ran.
			$nwsLink = 'http://graphical.weather.gov/xml/sample_products/browser_interface/ndfdXMLclient.php?lat='.$lat.'&lon='.$lng.'&product=time-series&begin='.$timeStart.'&end='.$timeEnd.'&Unit=e&maxt=maxt&mint=mint&temp=temp&pop12=pop12&qpf=qpf&rh=rh&sky=sky&wspd=wspd&wdir=wdir&wx=wx&icons=icons';
			$xml = simplexml_load_string(file_get_contents($nwsLink));
			//Ensures the processing will only occur if there is data returned.
			if($xml){
				//The content is scrubbed from the simplexml structure to make the data easier to work with.
				$content = $this->objectToArray(json_decode(json_encode($xml), true));
				$output['status'] = 'success';
				//The necessary location information, API feed source, and NwS weather page are provided.
				$output['results']['city'] = $results['results']['city'];
				$output['results']['state_region'] = $results['results']['state_region'];
				$output['results']['postal_code'] = $results['results']['postal_code'];
				$output['results']['lat'] = $lat;
				$output['results']['lng'] = $lng;
				$output['results']['nws_xml'] = $nwsLink;
				$output['results']['nws_page'] = $content['data']['moreWeatherInformation'];
				//NWS feeds have a funky time matrix that can be tough to decipher.  This part goes through and extracts the necessary layout keys for later use.
				//The tTimes array will have repeititive use throughout the code.
				$tTimes = array('start_valid_time', 'end_valid_time');
				$timeLayout = array();
				foreach($content['data']['time-layout'] as $tl){
					$layoutKey = $tl['layout-key'];
					foreach($tTimes as $tt){
						//Values with underscores are easier to read with JSON, but the feed uses dashes.  This section replaces the underscores with the dashes to better perform the search in the time matrix.
						$searchFor = str_replace('_', '-', $tt);
						if(array_key_exists($searchFor, $tl)){
							$timeLayout[$layoutKey][$tt] = $tl[$searchFor];
						}
					}
				}
				//Puts the data paramaters into a different variable so it's easier to sort through in the remainder of the function.
				$params = $content['data']['parameters'];
				//The weather conditions icons are extracted from the code, and the appropriate start and end times are added 
				$citl = $params['conditions-icon']['@attributes']['time-layout'];
				$iCount = 0;
				foreach($params['conditions-icon']['icon-link'] as $i){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $i;
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$citl])){
							$input[$tt] = $timeLayout[$citl][$tt][$iCount];
						}
					}
					$output['results']['data']['icon'][] = $input;
					$iCount++;
				}
				//The weather conditions attributes are extracted from the code, and the appropriate start and end times are added. 
				$wCount = 0;
				$wtl = $params['weather']['@attributes']['time-layout'];
				foreach($params['weather']['weather-conditions'] as $weather){
					$input['value'] = '';
					//Each attribute is put into a string
					if(count($weather) > 0){
						if(count($weather['value']['@attributes']) > 0){
							foreach($weather['value']['@attributes'] as $a){
								$input['value'] .= $a.' ';
							}
						}
					}
					//Each tTime value is gone through to ensure the necessary time values are added.
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wtl])){
							$input[$tt] = $timeLayout[$wtl][$tt][$wCount];
						}
					}
					$output['results']['data']['weather_conditions'][] = $input;
					$wCount++;
				}
				//Each temperature type is extracted from the code, and the appropriate start and end times are added. 
				foreach($params['temperature'] as $temp){
					//The temperature type and time layout is determined.
					$type = $temp['@attributes']['type'].'_temp';
					$tl = $temp['@attributes']['time-layout'];
					$tCount = 0;
					foreach($temp['value'] as $t){
						//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
						$input['value'] = $t.'&deg;F';
						foreach($tTimes as $tt){
							if(array_key_exists($tt, $timeLayout[$tl])){
								$input[$tt] = $timeLayout[$tl][$tt][$tCount];
							}
						}
						$output['results']['data'][$type][] = $input;
						$tCount++;
					}
				}
				//The precipitation attributes are extracted from the code, and the appropriate start and end times are added. 
				$ptl = $params['precipitation']['@attributes']['time-layout'];
				$pCount = 0;
				foreach($params['precipitation']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.' inches';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ptl])){
							$input[$tt] = $timeLayout[$ptl][$tt][$pCount];
						}
					}
					$output['results']['data']['precipitation'][] = $input;
					$pCount++;
				}
				//The cloud cover attributes are extracted from the code, and the appropriate start and end times are added. 
				$ctl = $params['cloud-amount']['@attributes']['time-layout'];
				$cCount = 0;
				foreach($params['cloud-amount']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ctl])){
							$input[$tt] = $timeLayout[$ctl][$tt][$cCount];
						}
					}
					$output['results']['data']['cloud_cover'][] = $input;
					$cCount++;
				}
				//The 12 hour precipitation probability attributes are extracted from the code, and the appropriate start and end times are added. 
				$ptl = $params['probability-of-precipitation']['@attributes']['time-layout'];
				$pCount = 0;
				foreach($params['probability-of-precipitation']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ptl])){
							$input[$tt] = $timeLayout[$ptl][$tt][$pCount];
						}
					}
					$output['results']['data']['12_hour_precip_prob'][] = $input;
					$pCount++;
				}
				//The humidity attributes are extracted from the code, and the appropriate start and end times are added. 
				$htl = $params['humidity']['@attributes']['time-layout'];
				$hCount = 0;
				foreach($params['humidity']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$htl])){
							$input[$tt] = $timeLayout[$htl][$tt][$hCount];
						}
					}
					$output['results']['data']['humidity'][] = $input;
					$hCount++;
				}
				//The wind direction attributes are extracted from the code, and the appropriate start and end times are added. 
				$wdtl = $params['direction']['@attributes']['time-layout'];
				$wdCount = 0;
				foreach($params['direction']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $this->degreeToDirection($value);
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wdtl])){
							$input[$tt] = $timeLayout[$wdtl][$tt][$wdCount];
						}
					}
					$output['results']['data']['wind_dir'][] = $input;
					$wdCount++;
				}
				//The wind speed attributes are extracted from the code, and the appropriate start and end times are added. 
				$wstl = $params['wind-speed']['@attributes']['time-layout'];
				$wsCount = 0;
				foreach($params['wind-speed']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $this->knotsToMiles($value). ' MPH';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wstl])){
							$input[$tt] = $timeLayout[$wstl][$tt][$wsCount];
						}
					}
					$output['results']['data']['wind_speed'][] = $input;
					$wsCount++;
				}
				//The field names are extracted from the array to populate into the database.
				$fields = array();
				foreach($output['results'] as $k=>$v){
					$fields[] = $k;
				}
				//Tmestamp and forecast length are added to the fields
				$fields[] = 'timestamp';
				$fields[] = 'forecast_length';
				//The values variable is formatted as an array for database insertion.g
				$values = $output['results'];
				$values['timestamp'] = REQUEST_TIME;
				$values['forecast_length'] = $endString;
				//Insertion query is run
				if($this->useMongo){
					$values['postal_code'] = floatval($values['postal_code']);
					$values['timestamp'] = floatval($values['timestamp']);
					$mGeo->geolocation_weather_cache->insert($values);
				}
				else{
					//The data gets JSON encoded for database storage.
					$values['data'] = json_encode($values['data']);
					db_insert('geolocation_weather_cache')->fields($fields)->values($values)->execute();
				}
				$output['msg'] = 'Got from NWS directly';
			}
		}
		//The output is converted to JSON unless otherwise specified, and then returned.
		if($isJson){
			$output = json_encode($output);
		}

		return $output;
	}

	/**
	*The ipFind function provides an IP Address lookup that can account for x-forwarded-for environments, and provides for a default if the application is being run in a local dev environment.
	*@return The IP address of the user (or Kirkland/Redmond, WA based IP for local dev environments).
	*/
	protected function ipFind($ip = ''){
		if($ip == ''){
			$http_headers = apache_request_headers();
			if($http_headers["X-Forwarded-For"] != ''){
				$ip = $http_headers["X-Forwarded-For"];
			}
			else{
				$ip = $_SERVER['REMOTE_ADDR'];
			}
		}
		$convert = array('127.0.0.1', '::1');
		if(in_array($ip, $convert)){
			$ip = '24.19.187.86';
		}
		return $ip;
	}

	/**
	*The ipCorrect function scrubs the inputted data to ensure city, state/region, and postal code are present, and will update the database if called for.
	*@param array $row
	*@param boolean $update default is true.
	*@return The $row array is passed back with corrected values as needed.
	*/
	protected function ipCorrect($row, $update = false){
		//Values checked for are in an array, and checked against the inputted array for a blank value.
		$checkFor = array('city','state_region','postal_code');
		$correctIt = false;
		foreach($checkFor as $c){
			if($row[$c] == '' || is_null($row[$c])){
				$correctIt = true;
				break;
			}
		}
		//If a blank value was detected, the system will call up the getAddressByGeo function to perform a lookup via the Google Geolocator API.
		if($correctIt){
			$geo = $row['lat'].','.$row['lng'];
			//For more info, see the getAddressByGeo function.
			$getNew = $this->getAddressByGeo($geo, false);
			$insert = array();
			//The updated values are inserted into the $row and $insert arrays
			foreach($checkFor as $c){
				$row[$c] = $getNew['results'][$c];
				$insert[$c] = $getNew['results'][$c];
			}
			//If an update is indicated, the system will automatically update the internal database with the new information.
			if($update){
				if($this->useMongo){
					$mGeo = $this->mongoDb('geolocation');
					$mGeo->geolocation_cities->update(array('id'=>$row['id']), $insert);
				}
				else{
					$q = db_update('geolocation_cities')->fields($insert)->condition('id', $row['id'], '=')->execute();
				}
				
			}
		}
		//Data is returned.
		return $row;
	}

	/**
	*The knotsToMiles function converts nautical miles into standard miles.  It is used in conjunction with the getWeather function.
	*@param string $speed
	*@return The conversion from knots to miles rounded to the nearest 100th.
	*/
	protected function knotsToMiles($speed){
		return round(($speed*1.15078), 2);
	}

	/**
	*The mongoDb function ensures that only one mongodb object is declared at one time, and returns the connection to the specified database collection.
	*@param string $db, default is geolocation
	*@return the database object for the specified collection.
	*/
	protected function mongoDb($db = 'geolocation'){
		if(!is_object($this->mdb)){
			$this->mdb = new Mongo();
		}
		return $this->mdb->{$db};
	}

	/**
	*The objectToArray function converts a data object into an associative array.
	*@param object/array $data
	*@return the $data array in associative array format.
	*/
	protected function objectToArray($data){
	    if (is_array($data) || is_object($data)){
	        $result = array();
	        foreach ($data as $key => $value){
	            $result[$key] = $this->objectToArray($value);
	        }
	        return $result;
	    }
	    return $data;
	}
}