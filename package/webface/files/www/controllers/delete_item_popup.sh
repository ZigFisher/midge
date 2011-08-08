#!/usr/bin/haserl

	item=$FORM_item
	action=${FORM_action:-question}
	subsys=${FORM_subsys:-""}
	export iface=${FORM_iface}

	render_form_header delete_item

	render_table_title "Question" 2
	case "${action}" in
	"question")
		render_table_row_text "<br><font size='+1'>Are you sure to delete item?</font><br>"
		render_table_tr_td_open "align=center"
		render_js_button "Yes" "newloc='/?controller=$controller&item=${item}&action=del&popup=1&subsys=${subsys}&iface=${iface}'; window.location=newloc "
		render_js_button "No" "window.close()"
		render_table_tr_td_close
		;;
	"del")
		$kdb lrm "$FORM_item"
		render_table_row_text "<br><font size='+1'>Item deleted!</font><br>"
		[ -n "$subsys" ] && update_configs_and_service_reload "$subsys"
		render_js_refresh_parent
		render_js_close_popup 2000
		;;
	esac
	render_form_tail
	
	
# vim:foldmethod=indent:foldlevel=1
