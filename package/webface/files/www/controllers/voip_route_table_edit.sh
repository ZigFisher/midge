#!/usr/bin/haserl

	subsys="voip"

	item=$FORM_item

	eval_string="export FORM_$item=\"enabled=$FORM_enabled router_id=$FORM_router_id address=$FORM_address comment=$FORM_comment\""
	render_popup_save_stuff
	
	render_form_header voip_route_edit
	help_1="voip.route"
	help_2="voip.route.add"
	render_table_title "VoIP route edit"
	render_popup_form_stuff
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled
	
	# router_id
	tip=""
	desc="Router ID"
	validator="$tmtreq $validator_voip_router_id"
	render_input_field text "Router ID" router_id
	
	# address
	tip=""
	desc="Router address"
	validator="$tmtreq $validator_ipaddr"
	render_input_field text "Address" address
	
	# comment
	tip=""
	desc="Comment for this record"
	validator="$validator_comment"
	render_input_field text "Comment" comment	
	
	render_submit_field
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
