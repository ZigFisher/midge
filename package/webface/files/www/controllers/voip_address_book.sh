#!/usr/bin/haserl
	. lib/misc.sh

	subsys="voip"	
	frame=1

	handle_list_del_item "voip"	# restart service on item deletion
	
	eval `$kdb -qqc list sys_voip_address_*`
	render_form_header voip_address

	render_list_line(){
		local lineno=$1
		local item="sys_voip_address_${lineno}"
		local target_img="<img src=img/blank.gif>"
		local style
		eval "var=\$$item"
		eval "$var"
		[ "x${enabled}x" = "xx" -o "x${enabled}x" = "x0x" ] && style="class='lineDisabled'"
		
		echo "<tr $style><td>$short_number</td><td>$complete_number</td><td>$comment</td><td>"
		
		render_list_btns voip_address_book_edit "$item" "subsys=$subsys"
		echo '</td></tr>'
	}

	render_list_header voip_address_book sys_voip_address_ "" "Short number" "Complete number" "Comment"
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
