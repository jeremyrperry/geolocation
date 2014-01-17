//This JavaScritp file is to populate the latitude and longitude fields with the user's HTML5 geocoordinates.
jQuery(document).ready(function(){
	jQuery('.lat_lng_lookup').click(function(){
		if(jQuery(this).attr('checked')){
			navigator.geolocation.getCurrentPosition(function(position){
	            jQuery('.lat').val(position.coords.latitude);
	            jQuery('.lng').val(position.coords.longitude);
	        });
		}
		else{
			jQuery('.lat').val('');
			jQuery('.lng').val('');
		}
	});
});