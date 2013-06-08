jQuery(document).ready(function(){
		jQuery("#lookup").click(function(){
			var ipAddress = jQuery("#ip_address").val();
			var postData = 'ajax=true&ip_address='+ ipAddress;
			jQuery.post('index.php', postData, function(data){
				jQuery("#results").html(data);
				jQuery("#ip_export").attr("href", "export.php?ip_address="+ipAddress);
			});
		});
	});