#!/usr/bin/haserl

	iface_proto=$FORM_iface_proto
	del_iface=$FORM_del_iface
	phys_iface=$FORM_phys_iface
	vlan_id=$FORM_vlan_id

	eval `kdb -qq ls sys_ifaces`

	get_next_iface() {
		local n=0
		local name
		local notfound
		case $1 in
			pptp|pppoe) name=$1;;
			bonding)	name=bond;;
			bridge)		name=br;;
		esac

		while [ 1 ]; do
			notfound=y
			for i in $sys_ifaces; do
				if [ ${name}${n} = $i ]; then notfound=n; break; fi
			done
			if [ $notfound = y ]; then
				echo ${name}${n}
				break;
			else
				n=$(($n+1))
			fi
		done
	}

	if [ $REQUEST_METHOD = POST ]; then
		if [ -n "$iface_proto" ]; then
			iface=`get_next_iface $iface_proto`
			iface_add $iface
			ok_str="Interface <b>$iface</b> added, reloading page"
			render_save_message
			render_js_refresh_window 1000
		elif [ -n "$del_iface" ]; then
			iface_del $del_iface
			del_iface=""
			ok_str="Interface deleted, reloading page"
			render_save_message
			render_js_refresh_window 1000
		elif [ -n "$phys_iface" ]; then
			iface_proto="vlan"
			realiface="${phys_iface}.${vlan_id}"
			depend_on=$phys_iface
			iface_add "${phys_iface}v${vlan_id}"
			ok_str="VLAN interface ${phys_iface}v${vlan_id} added, reloading page"
			render_save_message
			render_js_refresh_window 1000
		fi
	fi

	# Add interface
	render_form_header ifaces
	render_table_title "Add dynamic interface" 2

	desc="Please select interface protocol"
	validator='tmt:invalidindex=0 tmt:message="Please select protocol"'
	render_input_field select "Protocol" iface_proto bad "Please select interface protocol" bridge Bridge pppoe PPPoE pptp PPtP  bonding Bonding # ipsec IPSec

	render_submit_field Add
	render_form_tail
	
	
	# Add VLAN
	render_form_header ifaces
	help_1="vlan"
	help_2=""
	render_table_title "Add VLAN interface" 2
	
	desc=""
	validator='tmt:invalidindex=0 tmt:message="Please select interface"'
	for i in $sys_ifaces; do
		case $i in
			*v*)	;;
			*)	params="$params $i $i";;
		esac
	done
	render_input_field select "Physical interface" phys_iface bad "Please select interface" $params
	
	desc=""
	validator="$tmtreq tmt:pattern='positiveinteger' tmt:minnumber=0 tmt:maxnumber=4096 tmt:message='VLAN ID is a positive integer betwen 0 and 4096'"
	render_input_field text "VLAN ID " vlan_id

	render_submit_field "Add VLAN"
	render_form_tail
	

	# Delete interface
	render_form_header ifaces
	help_1="ifaces"
	help_2="ifaces.del"
	render_table_title "Delete dynamic interface" 2

	desc="Please select interface"
	validator='tmt:invalidindex=0 tmt:message="Please select interface"'
	params=""
	for i in $sys_ifaces; do
		case $i in
			*v*)		params="$params $i $i";;
			eth*|dsl*|E1*)	;;
			*)		params="$params $i $i";;
		esac
	done
	render_input_field select "Interface" del_iface bad "Please select interface" $params

	render_submit_field Delete
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
