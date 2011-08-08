#!/usr/bin/haserl

	subsys="voip"

	item=$FORM_item

	eval_string="export FORM_$item=\"enabled=$FORM_enabled short_number=$FORM_short_number complete_number='$FORM_complete_number' comment=$FORM_comment\""
	render_popup_save_stuff
	
	render_form_header voip_address_edit
	help_1="voip.address"
	help_2="voip.address.add"
	render_table_title "VoIP adress book edit"
	render_popup_form_stuff
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled
	
	# short_number
	tip=""
	desc="Short number for speed dialing"
	validator="$tmtreq $validator_voip_short_number"
	render_input_field text "Short number" short_number
	
	# complete_number
	tip=""
	desc="Complete telephone number"
	validator="$tmtreq $validator_voip_complete_number"
	render_input_field text "Complete number" complete_number
	
	# comment
	tip=""
	desc="Comment for this record"
	validator="$validator_comment"
	render_input_field text "Comment" comment	
	
	render_submit_field
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
