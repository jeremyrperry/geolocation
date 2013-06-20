function geoUpdate(campaignId, fileName){
	var postData = 'campaign_id='+campaignId;
	alert(postData);
	/*
	jQuery.post('/geolocation/location_update', postData, function(data){
		var jsonData = jQuery.parseJSON(data);
		jQuery('#geo_upload_msg').append(data);
		if(jsonData.status == 'success'){
			jQuery('#geo_upload_msg').append('<br />Updating complete.');
			window.location.href = '/geolocation/update_exports?campaign_id='+campaignId+'&file_name='+fileName;
		}
	});
*/
}