#!/usr/bin/haserl
	
	kdb_vars="int:svc_dns_tcpclients bool:svc_dns_enabled str:svc_dns_forwarder1 str:svc_dns_forwarder2"
	subsys="dns_server"

	render_save_stuff

	eval `kdb -qq ls svc_dns*`
	render_form_header dns 
	render_table_title "DNS Settings" 2 

#	# svc_dns_tcpclients
#	desc="Please enter number TCP clients"
#	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
#	render_input_field text "Max TCP Clients" svc_dns_tcpclients

	# svc_dns_enabled
	tip=""
	desc="Check this item if you want use DNS server on your router"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable DNS server" svc_dns_enabled

	# svc_dns_options_tmp
	#tip=""
	#desc=""
	#validator='tmt:required="true"'
	#render_input_field checkbox "TMP" svc_dns_options_tmp

	tip=""
	desc="Forward queries to DNS server"
	validator=$validator_dnszone
	render_input_field text "Forwarder DNS 1" svc_dns_forwarder1
	
	tip=""
	desc="Forward queries to DNS server"
	validator=$validator_dnszone
	render_input_field text "Forwarder DNS 2" svc_dns_forwarder2

	render_submit_field
	render_form_tail

	# dns zone list
	render_form_header dns_zonelist
	help_1="dns_server"
	help_2="dns_server.dns_zones" 
	render_table_title "Zones" 2 
	render_iframe_list "dns_zonelist"
	render_form_tail

	
# vim:foldmethod=indent:foldlevel=1
