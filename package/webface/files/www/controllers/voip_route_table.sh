#!/usr/bin/haserl
	. lib/misc.sh
	
	subsys="voip"
	frame=1

	handle_list_del_item "voip"	# restart service on item deletion
	
	eval `$kdb -qqc list sys_voip_route_*`
	render_form_header voip_route
	
	render_list_line(){
		local lineno=$1
		local item="sys_voip_route_${lineno}"
		local target_img="<img src=img/blank.gif>"
		local style
		eval "var=\$$item"
		eval "$var"
		[ "x${enabled}x" = "xx" -o "x${enabled}x" = "x0x" ] && style="class='lineDisabled'"
		
		echo "<tr $style><td>$router_id</td><td>$address</td><td>$comment</td><td>"
		
		render_list_btns voip_route_table_edit "$item" "subsys=$subsys"
		echo '</td></tr>'
	}

	render_list_header voip_route_table sys_voip_route_ "" "Router identifier" "Address" "Comment"
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
