#!/usr/bin/haserl
	
	item=$FORM_item
	iface=$FORM_iface
	subsys="network.$iface"
	
	# generate list of available classes
	for key in `kdb kls sys_iface_${iface}_qos_htb_class_*`; do
		val=`kdb get $key`
		eval "$val"
		CLASSES="$CLASSES $classid $name"
	done
	unset name enabled parent classid rate ceil
	
	eval_string="export FORM_$item=\"name=$FORM_name enabled=$FORM_enabled prio=$FORM_prio proto=$FORM_proto src=$FORM_src dst=$FORM_dst src_port=$FORM_src_port dst_port=$FORM_dst_port flowid=$FORM_flowid\""
	render_popup_save_stuff

	render_form_header qos_filter_edit
	help_1="htb"
	help_2="htb_filter_add"
	render_table_title "QoS filter edit" 2
	render_popup_form_stuff
	
	render_input_field hidden iface iface "$iface"
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled	
	
	# name
	desc="Name of filter"
	validator="$tmtreq $validator_rulename"
	render_input_field text "Short name" name
		
	# prio
	desc="Rule priority"
	default="1"
	validator="$tmtreq $validator_prio"
	tip="Prio can be any positive integer value.<br><b>Examples:</b> 1, 10, 17"
	render_input_field text "Prio" prio
	
	# proto
	desc="Protocol"
	render_input_field select "Protocol" proto "any" "any" "tcp" "TCP" "udp" "UDP" "icmp" "ICMP"
	
	iptip="Address can be either a network IP address (with /mask), or a plain IP address.<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>"
	
	# src
	desc="Source IP"
	default="0.0.0.0/0"
	tip=$iptip
	validator="$tmtreq $validator_ipnet_ipt"
	render_input_field text "Src" src
	
	# dst
	desc="Destination IP"
	default="0.0.0.0/0"
	tip=$iptip
	validator="$tmtreq $validator_ipnet_ipt"
	render_input_field text "Dst" dst
	
	porttip="Port number"
	
	# src_port
	desc="Source port"
	default="any"
	tip=$porttip
	validator="$tmtreq $validator_ipport"
	render_input_field text "Src port" src_port
	
	# dst_port
	desc="Destination port"
	default="any"
	tip=$porttip
	validator="$tmtreq $validator_ipport"
	render_input_field text "Dst port" dst_port

	# flowid
	desc="Put matching packets in this class"
	render_input_field select "Class" flowid $CLASSES
	
	render_submit_field
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
