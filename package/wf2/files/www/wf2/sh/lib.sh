update_configs_and_service_reload(){
	local subsys="$1"
	local s
	local service
	for s in $subsys; do 
		update_configs $s
		[ "$ERROR_MESSAGE" ] && return
	done
	fail_str="Update config failed: $ERROR_DETAIL"
	[ "$ERROR_MESSAGE" ] && return
	for service in $subsys; do
		service_reload $service 2>&1 | $LOGGER
	done
}
