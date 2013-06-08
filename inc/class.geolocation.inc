<?php
class geolocation{

	private $db;

	function __construct($locaton = 'geolocation'){
		db_set_active('geolocation');
		$this->db = Database::getConnection();
	}

	function __destruct(){
		db_set_active('default');
	}

	private function esc($val){
		return $this->db->escapeField($val);
	}

	public function getIpv4($ip){
		$ipArr = explode('.', $ip);
		$output =  (16777216 * $ipArr[0])+(65536 * $ipArr[1]) + (256 * $ipArr[2])+ $ipArr[3];
		return $output;
	}

	public function getIp($ipAddress, $json = true){
		$output = array();
		$ipAddress = $this->esc($ipAddress);
		$query = "select loc_id from ip_locations where INET_ATON('".$ipAddress."') between start_ip_num and end_ip_num limit 1";
		$q = $this->db->query($query);
		if(!$q){
			$output['status'] = 'error';
			$output['results'] = 'Query failure: '.$this->db->error;
		}
		else{
			$r = $q->fetchAssoc();
			$locId = $r['loc_id'];
			$query2 = "select * from cities where id = '".$locId."'";
			$q2 = $this->db->query($query2);
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
		$postalCode = $this->esc($postalCode);
		$query = "select * from cities where postal_code = '".$postalCode."'";
		$q = $this->db->query($query);
		if($q){
			$r = $q->fetchAssoc();
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			$output['status'] = 'error';
			$output['results'] = $this->db->error;
		}
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}

	public function getCityPostalByGeo($val, $json = true){
		$coords = explode(',', $val);
		$lat = $coords[0];
		$lon = $coords[1];
		$query = "select *, round(degrees(acos(sin(radians(".$lat."))*sin(radians(lat))+cos(radians(".$lat."))*cos(radians(lat))*cos(radians(".$lon." - lng)))) *69.09) as distance from cities  where postal_code != '' order by distance asc limit 1";
		$q = $this->db->query($query);
		if($q){
			$r = $q->fetchAssoc();
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			$output['status'] = 'error';
			$output['results'] = $this->db->error;
		}
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}

	public function getAddressByGeo($geo, $isJson = true){
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?latlng='.$geo.'&sensor=false';
		$json = json_decode(file_get_contents($url), true);
		$results['address'] = $json['results'][0]['address_components'][0]['long_name'].' '.$json['results'][0]['address_components'][1]['short_name'];
		$results['city'] = $json['results'][0]['address_components'][3]['long_name'];
		$results['state_region'] = $json['results'][0]['address_components'][5]['short_name'];
		$results['postal_code'] = $json['results'][0]['address_components'][7]['short_name'];
		$results['country'] = $json['results'][0]['address_components'][6]['short_name'];
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

	public function error($json = true){
		$output['status'] = 'error';
		$output['msg'] = 'invalid request';
		if($json){
			$output = json_encode($output);
		}
		return $output;
	}
}