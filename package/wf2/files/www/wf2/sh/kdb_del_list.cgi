#!/usr/bin/haserl
<?
	. /etc/templates/lib
	. /www/lib/service.sh
	. ./lib.sh
	
	/usr/bin/kdb lrm "$FORM_item"
	
	[ -n "$FORM_subsystem" ] && update_configs_and_service_reload "$FORM_subsystem"
	
	echo ""
?>
