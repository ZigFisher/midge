#!/usr/bin/haserl
<?
	. /etc/templates/lib
	. /www/lib/service.sh
	. ./lib.sh
	
	[ -n "$FORM_subsystem" ] && update_configs_and_service_reload "$FORM_subsystem"
	
	/usr/bin/kdb rm "$FORM_item"
	
	echo ""
?>
