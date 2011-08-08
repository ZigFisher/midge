#!/usr/bin/haserl
<?
	. /etc/templates/lib
	. /www/lib/service.sh
	. ./lib.sh
	
	KDB_PARAMS=""
	
	kdb_set_val() {
		KDB_PARAMS="${KDB_PARAMS} : set '$1' "
	}
	
	kdb_commit() {
		eval "/usr/bin/kdb ${KDB_PARAMS}"
		return $?
	}
	
IFS='
'
	for variable in $(env |grep FORM); do
		[ $(echo $variable |grep -c "FORM_SESSIONID") -eq 1 ] && continue
		[ $(echo $variable |grep -c "FORM_subsystem") -eq 1 ] && continue
		variable=$(echo $variable |sed s/FORM_//)
		kdb_set_val "$variable"
	done
IFS=' '

	kdb_commit
	
	[ -n "$FORM_subsystem" ] && update_configs_and_service_reload "$FORM_subsystem"
	
	echo ""
?>
