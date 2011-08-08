#!/usr/bin/haserl

	frame=1	
	iface=${FORM_iface}
	subsys="network.$iface"
	
	handle_list_del_item $subsys
	
	eval `$kdb -qqc list sys_iface_${iface}_qos_htb_filter_*`
	render_form_header qos_filter

	# get name of given classid
	get_parent(){
		local name enabled parent classid rate ceil
		unset name
		parent_name="NONE"
		eval `kdb get sys_iface_${iface}_qos_htb_class_*|grep classid=$1`
		[ -n "$name" ] && parent_name=$name
	}

	render_list_line(){
		local lineno=$1
		local item="sys_iface_${iface}_qos_htb_filter_${lineno}"
		local target_img="<img src=img/blank.gif>"
		local style
		eval "var=\$$item"
		eval "$var"
		
		get_parent $flowid
		
		[ "x${enabled}x" = "xx" ] && style="class='lineDisabled'"
		
		echo "<tr $style><td>$lineno</td><td>$name</td><td>$prio</td><td>$proto</td><td>$src</td><td>$dst</td><td>$src_port</td><td>$dst_port</td><td>$parent_name</td><td>"
		render_list_btns qos_filter_edit "$item" "iface=$iface"
		echo '</td></tr>'
	}
	
	render_list_header qos_filter sys_iface_${iface}_qos_htb_filter_ "iface=$iface" "No" "Name" "Prio" "Proto" "Src addr" "Dst addr" "Src port" "Dst port" "Class"
	
	render_list_cycle_stuff

	render_form_tail

# vim:foldmethod=indent:foldlevel=1
