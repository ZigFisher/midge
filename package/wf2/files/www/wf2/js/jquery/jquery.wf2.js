/* Validator methods */

/* IP address */
jQuery.validator.addMethod("ipAddr", function(value, element) {
	return this.optional(element) ||
		/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/.test(value);
}, "Please enter correct IP address.");

/* IP port */
jQuery.validator.addMethod("ipPort", function(value, element) {
	return this.optional(element) || /^((\d)+|any)$/.test(value);
}, "Please enter correct IP port.");

/* IP address with optional port */
jQuery.validator.addMethod("ipAddrPort", function(value, element) {
	return this.optional(element) ||
		/^(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])(:\d+)?$/.test(value);
}, "Please enter correct IP address with optional port.");

/* IP port range */
jQuery.validator.addMethod("ipPortRange", function(value, element) {
	return this.optional(element) || /^((\d)+(:)?((\d)+)?|(:)((\d)+)|any)$/.test(value);
}, "Please enter correct IP port or port range.");

/* IP netmask */
jQuery.validator.addMethod("netmask", function(value, element) {
	return this.optional(element) ||
		/^(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])$/.test(value);
}, "Please enter correct ip netmask.");

/* IP address with optional netmask */
jQuery.validator.addMethod("ipNetMaskIptables", function(value, element) {
	return this.optional(element) ||
		/^[!]?(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(0?0?\d|[01]?\d\d|2[0-4]\d|25[0-5])(\/\d\d*)?$/.test(value);
}, "Please enter correct ip address with optional mask.");

/* MAC address */
jQuery.validator.addMethod("macAddr", function(value, element) {
	return this.optional(element) ||
		/^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$/.test(value);
}, "Please enter correct mac address.");

/* Domain name */
jQuery.validator.addMethod("domainName", function(value, element) {
	return this.optional(element) || /^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$/.test(value);
}, "Please enter correct domain name.");

/* DNS record domain */
jQuery.validator.addMethod("dnsRecordDomainOrIpAddr", function(value, element) {
	return this.optional(element) || /(^@$)|(^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$)|(^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$)$/.test(value);
}, "Please enter correct domain name.");

/* Domain name or IP address */
jQuery.validator.addMethod("domainNameOrIpAddr", function(value, element) {
	return this.optional(element) || /(^[a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?$)|(^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$)/.test(value);
}, "Please enter correct dns domain name or IP-address.");

/* map of slots for E1 */
jQuery.validator.addMethod("smap", function(value, element) {
	return this.optional(element) ||
		/^(([0-9]+,)|([0-9]+-[0-9]+(,)?)|([0-9]+$))+$/.test(value);
}, "Please enter correct slot map.");

/* VoIP's router id */
jQuery.validator.addMethod("voipRouterID", function(value, element) {
	return this.optional(element) || /^([0-9]){3}$/.test(value);
}, "Please enter correct Router ID.");

/* VoIP's router id with "*" */
jQuery.validator.addMethod("voipRouterIDWithSelf", function(value, element) {
	return this.optional(element) || /(^([0-9]){3}$)|(^\*$)/.test(value);
}, "Please enter correct Router ID.");

/* VoIP's FXO/FXS channel */
jQuery.validator.addMethod("voipChannel", function(value, element) {
	return this.optional(element) || /^([0-9]){2}$/.test(value);
}, "Please enter correct FXO/FXS channel.");

/* VoIP's registrar */
jQuery.validator.addMethod("voipRegistrar", function(value, element) {
	return this.optional(element) || /^sip:(([a-zA-Z0-9]+([a-zA-Z0-9\-\.]+)?)|((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])))$/.test(value);
}, "Please enter correct registrar name or it's IP-address, with sip: prefix.");

/* VoIP's SIP URI */
jQuery.validator.addMethod("voipSipUri", function(value, element) {
	return this.optional(element) || /^sip:([_a-zA-Z0-9](\.[_a-zA-Z0-9])?)+@([_a-zA-Z0-9]\.?)+/.test(value);
}, "Please enter correct user SIP URI, with sip: prefix.");

/* VoIP's short number */
jQuery.validator.addMethod("voipShortNumber", function(value, element) {
	return this.optional(element) || /^([0-9]){2}$/.test(value);
}, "Please enter 2-digit positive number.");

/* VoIP's complete number */
jQuery.validator.addMethod("voipCompleteNumber", function(value, element) {
	return this.optional(element) || /(^([0-9]{3}|[\*])([-,]*)([0-9]{2}|[\*])([-,]*)([-0-9,]*)$)|(^#sip:([_a-zA-Z0-9](\.[_a-zA-Z0-9])?)+@([_a-zA-Z0-9]\.?)+#$)/.test(value);
}, "Please enter correct complete number.");

/* VoIP's payload */
jQuery.validator.addMethod("voipPayload", function(value, element) {
	return this.optional(element) || /(^0x[0-9a-fA-F]+$)|(^[0-9]+$)/.test(value);
}, "Please enter correct payload type.");

/* PBO value */
jQuery.validator.addMethod("pbo", function(value, element) {
	return this.optional(element) || /^(\d+(:\d+)*)+$/.test(value);
}, "Please enter correct PBO value.");

/* Alphanumeric, underline and "-" only */
jQuery.validator.addMethod("alphanumU", function(value, element) {
	return this.optional(element) || /^[-_a-zA-Z0-9]+$/.test(value);
}, "Please enter alphanumeric and underline characters only (without spaces).");

/* Alphanumeric only */
jQuery.validator.addMethod("alphanum", function(value, element) {
	return this.optional(element) || /^[a-zA-Z0-9]+$/.test(value);
}, "Please enter alphanumeric characters only (without spaces).");

/* QoS bandwith */
jQuery.validator.addMethod("qosBandwith", function(value, element) {
	return this.optional(element) || /^([0-9]+(k|M)(bit|bps))$/.test(value);
}, "Please enter correct bandwith.");

/* ToS */
jQuery.validator.addMethod("tos", function(value, element) {
	return this.optional(element) ||
		/^(0x)([0-9a-fA-F])?([0-9a-fA-F])?$/.test(value);
}, "Please enter correct ToS (e.g., 0x5).");

/* Unique value among all elements specified with selector in parameter */
jQuery.validator.addMethod("uniqueValue", function(value, element, params) {
	if (this.optional(element)) {
        return true;
    }
	var unique = true;
	$(params).not(element).each(function(num, anotherValue) {
		if ($(anotherValue).val() == value) {
			unique = false;
			return false;
		}
	});
	return unique;
}, "Please enter unique values.");

/* value must be more than first parameter, and less than second parameter, parameters are selectors */
jQuery.validator.addMethod("dynamicRange", function(value, element, params) {
	if (this.optional(element)) {
        return true;
    }

    var val = parseInt(value, 10);
    var min = parseInt($(params[0]).val(), 10);
    var max = parseInt($(params[1]).val(), 10);

    return val >= min && val <= max;
}, "Value must be more than minimum and less than maximum.");

/*
 * Plugin name: len.
 *
 * Returns number of propertiens in object.
 */
(function($) {
	$.extend({
		len: function(object) {
			var num = 0;
			for (var i in object) {
				num++;
			}
			return num;
		}
	});
})(jQuery);


/*
 * Plugin name: addObjectWithProperty.
 *
 * Creates new object like { name: 'property', value: 'value' }
 * and adds it to array.
 *
 * array - destination array;
 * property - name of property;
 * value - value of property.
 */
(function($) {
	$.extend({
		addObjectWithProperty: function(array, property, value) {
			var object = new Object();
			object.name = property;
			object.value = value;
			array.push(object);
		}
	});
})(jQuery);


(function($) {
	/*
	 * Plugin name: setOptionsForSelect.
	 *
	 * Adds options elements to select element.
	 *
	 * options.options - list of options, may be:
	 *  - string with options, separated by space (option value = option name);
	 *  - array (option value == option name);
	 *  - hash ({optionValue: optionName});
	 * options.curValue - select option with this value;
	 * options.defaultValue - if no option is selected, select option with this value;
	 * options.doNotClear - dot not empty current select's list.
	 */
	$.fn.setOptionsForSelect = function(options) {
		var selectedIndex = -1;
		var defaultIndex = -1;
		var selectedItem;

		var selectObject = $(this);

		/* remove previous options */
		if (!options.doNotClear) {
			selectObject.empty();
		}

		/* if option's list is a string — convert it to hash */
		if (typeof options.options == "string") {
			var vals = options.options;
			options.options = {};
			$.each(vals.split(" "), function(num, value) {
				options.options[value] = value;
			});
		}

		/* if option's list is array — convert it to hash */
		if (options.options.constructor == Array) {
			var arr = options.options;
			options.options = {};
			$.each(arr, function(num, value) {
				/* values have to be strings */
				options.options[value + ""] = value + "";
			});
		}

		/* go though list of options */
		$.each(options.options, function(name, value) {
			/* convert value and name of option to string */
			name = name + "";
			value = value + "";

			/*
			 * Heh. In options list property name is the value of option, and
			 * property value is the text of option.
			 */
			var attrs = {"value": name};

			/* if current option should be selected */
			if (options.curValue == name) {
				selectedItem = name;
			}

			/* add option to select element */
			$.create("option", attrs, value).appendTo(selectObject);
		});

		/* find selectedIndex and defaultIndex */
		$("option", selectObject).each(function(idx) {
			this.value == selectedItem && (selectedIndex = idx);
			this.value == options.defaultValue && (defaultIndex = idx);
		});

		/* set selected index in select element */
		if (selectedIndex != -1) {
			$(selectObject).attr("selectedIndex", selectedIndex);
		}
		/* if nothing is selected — select default item */
		else if (defaultIndex != -1) {
			$(selectObject).attr("selectedIndex", defaultIndex);
		}
	},
	

	/*
	 * Plugin name: setSelectReadonly.
	 *
	 * Set/unset select element read-only.
	 * 
	 * state:
	 *  - true: disables select element and adds hidden field for sending value to server;
	 *  - false: enables select element and removes hidden field.
	 */
	$.fn.setSelectReadonly = function(state) {
		if (state) {
			$(this).attr("disabled", true);

            /* add hidden field if it doesn't exist */
            if ($($.sprintf("input[type=hidden][name=%s]", $(this).attr("name"))).length == 0) {
                $.create("input", {
					"type": "hidden",
					"name": $(this).attr("name"),
					"value": $(this).val()
				}).insertAfter(this);
            }
		} else {
			$(this).removeAttr("disabled");
			$($.sprintf("input[type=hidden][name=%s]", $(this).attr("name"))).remove();
		}
	}
})(jQuery);
