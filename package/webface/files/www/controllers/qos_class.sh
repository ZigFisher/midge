#!/usr/bin/haserl

	frame=1		
	iface=${FORM_iface}
	subsys="network.$iface"
	
	handle_list_del_item $subsys
	
	eval `$kdb -qqc list sys_iface_${iface}_qos_htb_class_*`
	render_form_header qos_class

	# get name of given classid
	get_parent(){
		local name enabled parent classid rate ceil qdisc
		unset name
		parent_name="NONE"
		eval `kdb get sys_iface_${iface}_qos_htb_class_*|grep classid=$1`
		[ -n "$name" ] && parent_name=$name
	}

	render_list_line(){
		local lineno=$1
		local item="sys_iface_${iface}_qos_htb_class_${lineno}"
		local target_img="<img src=img/blank.gif>"
		local style
		local _qdisc
		eval "var=\$$item"
		eval "$var"
		
		parent_name="root"
		[ "$parent" != "1:0" ] && get_parent $parent
		
		[ "x${enabled}x" = "xx" ] && style="class='lineDisabled'"	
		
		# replace '#' in qdisc with spaces
		_qdisc=`echo $qdisc |sed s/#/" "/g`

		echo "<tr $style><td>$lineno</td><td>$name</td><td>$parent_name</td><td>$rate</td><td>$ceil</td><td>$_qdisc</td><td>"
		render_list_btns qos_class_edit "$item" "iface=$iface"
		echo '</td></tr>'
	}
	
	render_list_header qos_class sys_iface_${iface}_qos_htb_class_ "iface=$iface" "No" "Name" "Parent" "Rate" "Ceil" "Qdisc"
	
	render_list_cycle_stuff

	render_form_tail

# vim:foldmethod=indent:foldlevel=1