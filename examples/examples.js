$(document).ready(function() {

	/* This is basic - uses default settings */

	$(".fancybox").fancybox({
		'hideOnContentClick': true,
		'transitionIn'	:	'elastic',
			'transitionOut'	:	'elastic',
			'speedIn'		:	600, 
			'speedOut'		:	200, 
			'overlayShow'	:	false
	});
	
	/* Using custom settings */
	
	// $("a#inline").fancybox({
	// 	'hideOnContentClick': true
	// });
	
});