function mr16hModuleName(pcislot) {
	return $.sprintf("%s%s%s", config.getOEM("MR16H_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)));
}

function mr17hModuleName(iface, pcislot) {
	/* config path */
	var confPath = $.sprintf("%s/%s/sg_private", config.getOEM("sg17_cfg_path"), iface);

	/* power status */
	var pwrPresence = config.getCachedOutput($.sprintf("/bin/cat %s/pwr_source", confPath));

	/* chip version */
	var chipVer = config.getCachedOutput($.sprintf("/bin/cat %s/chipver", confPath));

	var sfx;
	if (chipVer == "v1") {
		sfx = config.getOEM("MR17H_V1SFX");
	} else if (chipVer == "v2") {
		sfx = config.getOEM("MR17H_V2SFX");
	}

	if (pwrPresence == "1") {
		sfx += config.getOEM("MR17H_PWRSFX");
	}

	return $.sprintf("%s%s%s%s", config.getOEM("MR17H_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)), sfx);
}

Controllers.dsl = function(iface, pcislot, pcidev) {
	/* create page and set common settings */
	var page = this.Page();
	page.setHelpPage("dsl");

	/* fill select with rates */
	var rateList = function(select, first, last, step, cur) {
		var rates = "";
		for (var i = first; i <= last; i += step) {
			rates += i + " ";
		}
		rates = $.trim(rates);
		$(select).setOptionsForSelect({"options": rates, "curValue": cur});
	};

	/* status for MR16H */
	var sg16Status = function() {
		var c, field;
		c = page.addContainer("status");
		c.addTitle($.sprintf("%s (module %s) status", iface, mr16hModuleName(pcislot)));
		
		field = {
			"type": "html",
			"name": "link_state",
			"text": "Link state",
			"cmd": $.sprintf("/bin/cat %s/%s/state", config.getOEM("sg16_cfg_path"), iface)
		};
		c.addWidget(field);
	};

	/* settings for MR16H */
	var sg16Settings = function() {
		var c, field;
		c = page.addContainer("settings");
		c.addTitle($.sprintf("%s (module %s) settings", iface, mr16hModuleName(pcislot)));
		c.setSubsystem($.sprintf("dsl.%s.%s", pcislot, pcidev));
		
		/* available TCPAM values */
		var TCPAM = {
			"tcpam32": "TCPAM32",
			"tcpam16": "TCPAM16",
			"tcpam8": "TCPAM8",
			"tcpam4": "TCPAM4"
		};
		
		/* updates parameters */
		var onChangeSG16Code = function() {
			var cfg = $("#cfg").val();
			var annex = $("#annex").val();
			var mode = $("#mode").val();
			var code = $("#code").val();
			var rate = $("#rate").val();
			
			if (cfg == "preact" && annex == "F") {
				if (mode == "slave") {
					$("#code").setOptionsForSelect({"options": {"tcpam32": TCPAM["tcpam32"]}});
					$("#code").attr("readonly", true);
					
					$("#rate").setOptionsForSelect({"options": "automatic"});
					$("#rate").attr("readonly", true);
				} else {					
					$("#code").removeAttr("readonly");
					$("#rate").removeAttr("readonly");
					
					$("#code").setOptionsForSelect({
							"options": {"tcpam16": TCPAM["tcpam16"], "tcpam32": TCPAM["tcpam32"]},
							"curValue": code
					});
					
					/* update varibale's value */
					code = $("#code").val();
					
					if (code == "tcpam16") {
						rateList("#rate", 192, 2304, 64, rate);
					} else {
						rateList("#rate", 192, 5696, 64, rate);
					}
				}
			} else if (cfg == "preact") {
				$("#code").setOptionsForSelect({"options": {"tcpam16": TCPAM["tcpam16"]}});
				$("#code").attr("readonly", true);
				
				if (mode == "slave") {
					$("#rate").setOptionsForSelect({"options": "automatic"});
					$("#rate").attr("readonly", true);
				} else {
					$("#rate").removeAttr("readonly");
					rateList("#rate", 192, 2304, 64, rate);
				}
			} else {
				$("#code").removeAttr("readonly");
				$("#rate").removeAttr("readonly");
				
				$("#code").setOptionsForSelect({"options": TCPAM, "curValue": code});
				
				/* update varibale's value */
				code = $("#code").val();
				
				switch (code) {
					case "tcpam4":
						rateList("#rate", 64, 704, 64, rate);
						break;
					case "tcpam8":
						rateList("#rate", 192, 1216, 64, rate);
						break;
					case "tcpam16":
						rateList("#rate", 192, 3840, 64, rate);
						break;
					case "tcpam32":
						rateList("#rate", 256, 6016, 64, rate);
						break;
				}
			}
		};
		
		/* add parameters */
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
			"id": "mode",
			"text": "Mode",
			"descr": "Select DSL mode",
			"options": {"master": "Master", "slave": "Slave"},
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		var name = $.sprintf("sys_pcicfg_s%s_%s_rate", pcislot, pcidev);
		var rate = config.get(name);
		field = { 
			"type": "select",
			"name": name,
			"id": "rate",
			"text": "Rate",
			"descr": "Select DSL line rate",
			"options": rate
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev),
			"id": "code",
			"text": "Coding",
			"descr": "Select DSL line coding",
			"options": TCPAM,
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_cfg", pcislot, pcidev),
			"id": "cfg",
			"text": "Config",
			"descr": "Select DSL configuration mode",
			"options": {"local": "local", "preact": "preact"},
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
			"id": "annex",
			"text": "Annex",
			"descr": "Select DSL Annex",
			"options": {"A": "Annex A", "B": "Annex B", "F": "Annex F"},
			"onChange": onChangeSG16Code
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_crc", pcislot, pcidev),
			"text": "CRC",
			"descr": "Select DSL CRC length",
			"options": {"crc32": "CRC32", "crc16": "CRC16"}
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
			"text": "Fill",
			"descr": "Select DSL fill byte value",
			"options": {"fill_ff": "FF", "fill_7e": "7E"}
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_inv", pcislot, pcidev),
			"text": "Inversion",
			"descr": "Select DSL inversion mode",
			"options": {"normal": "off", "invert": "on"}
		};
		c.addWidget(field);
		
		c.addSubmit();
		
		onChangeSG16Code();
	};
	
	/* return title for MR17H */
	var getSg17Title = function() {
		return $.sprintf("%s (module %s, %s compatibility) ", iface, 
				mr17hModuleName(iface, pcislot), (chipVer == "v1") ? "base" : "extended");
	};
	
	/* show status for 17 series */
	var sg17Status = function(dsl17status) {
		var c, field;
		c = page.addContainer("status", {"clear": true});
		c.addTitle(getSg17Title() + "status");
		
		field = {
			"type": "html",
			"name": "link_state",
			"text": "Link state",
			"str": dsl17status.link.link_state == "1" ? "online" : "offline"
		};
		c.addWidget(field);
		
		/* power present */
		if (dsl17status.pwr.presence == "1") {
			field = {
				"type": "html",
				"name": "pwrUnb",
				"text": "Power balance",
				"str": dsl17status.pwr.unb == "0" ? "balanced" : "unbalanced"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "pwrOvl",
				"text": "Power overload",
				"str": dsl17status.pwr.ovl == "0" ? "no overload" : "overload"
			};
			c.addWidget(field);
		}
		
		/* online */
		if (dsl17status.link.link_state == "1") {
			field = {
				"type": "html",
				"name": "actualRate",
				"text": "Actual rate",
				"str": dsl17status.link.rate
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "actualLineCode",
				"text": "Actual line code",
				"str": dsl17status.link.tcpam
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "actualClockMode",
				"text": "Actual clock mode",
				"str": dsl17status.link.clkmode
			};
			c.addWidget(field);
			
			/* statistics */
			field = {
				"type": "html",
				"name": "snrMargin",
				"text": "SNR margin",
				"descr": "Signal/Noise ratio margin",
				"str": dsl17status.link.statistics_row.split(" ")[0]
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "loopAttn",
				"text": "Loop attenuation",
				"str": dsl17status.link.statistics_row.split(" ")[1]
			};
			c.addWidget(field);
		}
		
		/* PBO */
		field = {
			"type": "html",
			"name": "pboMode",
			"text": "PBO",
			"descr": "Power backoff",
			"str": dsl17status.pbo.mode
		};
		c.addWidget(field);
		
		if (dsl17status.pbo.mode == "Forced") {
			field = {
				"type": "html",
				"name": "pboVal",
				"text": "PBO values",
				"descr": "Power backoff values",
				"str": dsl17status.pbo.val + " dB"
			};
			c.addWidget(field);
		}
	};
	
	/* show settings for MR17H */
	var sg17Settings = function() {
		var c, field;
		c = page.addContainer("settings");
		c.addTitle(getSg17Title() + "settings");
		c.setSubsystem($.sprintf("dsl.%s.%s", pcislot, pcidev));
		
		/* control mode change (manual or by EOCd) */
		var onCtrlChange = function() {
			if ($("#ctrl").val() == "manual") {
				$(".widgetEocd, .widgetEocdMaster, .widgetMaster").parents("tr").remove();
				addManualWidgets();
			} else {
				$(".widgetManual, .widgetManualMaster, .widgetMaster").parents("tr").remove();
				addEocdWidgets();
			}
		};

		/* dsl mode change */
		var onModeChange = function() {
			if ($("#mode").val() == "slave") {
				$(".widgetManualMaster, .widgetMaster").parents("tr").remove();
			} else {
				addManualMasterWidgets();
			}
		};

		/* EOCd master change */
		var onEocdMasterChange = function() {
			if ($("#eocdMaster").val() == "0") {
				$(".widgetEocdMaster, .widgetMaster").parents("tr").remove();
			} else {
				addEocdMasterWidgets();
			}
		};

		/* add or remove pboval field depending on pbomode value */
		var setPboval = function() {
			/* If PBO is active, add text widget to enter it's value */
			if ($("#pbomode").attr("checked") == true && $("#pboval").length == 0) {
				field = {
					"type": "text",
					"name": $.sprintf("sys_pcicfg_s%s_%s_pboval", pcislot, pcidev),
					"id": "pboval",
					"validator": {"required": true, "pbo": true}
				};
				c.addSubWidget(field, {"type": "insertAfter", "anchor": "#pbomode"});
			} else if ($("#pbomode").attr("checked") == false) {
				$("#pboval").remove();
			}
		};

		/* add or remove mrate field depending on rate value */
		var setMrate = function() {
			/* if rate is "other", add text widget to enter rate value */
			if ($("#rate").val() == "-1" && $("#mrate").length == 0) {
				var field = {
					"type": "text",
					"name": $.sprintf("sys_pcicfg_s%s_%s_mrate", pcislot, pcidev),
					"id": "mrate",
					"validator": {"required": true, "min": 0},
					"cssClass": "widgetManualMaster",
					"onChange": function() {
						$("#mrate").val(getNearestCorrectRate(chipVer, $("#tcpam").val(),
								$("#mrate").val()))
					},
					"valueFilter": function(value) {
						return getNearestCorrectRate(chipVer, $("#tcpam").val(), value);
					}
				};
				c.addSubWidget(field, {"type": "insertAfter", "anchor": "#rate"});
			/* otherwise, remove it */
			} else if ($("#rate").val() != "-1") {
				$("#mrate").remove();
			}
		};

		/* add, if not exist, widgets for manual mode */
		var addManualWidgets = function() {
			if ($(".widgetManual").length > 0) {
				return;
			}

			/* mode */
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_mode", pcislot, pcidev),
				"id": "mode",
				"text": "Mode",
				"descr": "DSL mode.",
				"options": {"master": "Master", "slave": "Slave"},
				"cssClass": "widgetManual",
				"onChange": onModeChange
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#ctrl").parents("tr")});

			/* add widget for power-capable interfaces */
			if (pwrPresence == 1) {
				/* pwron */
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_pwron", pcislot, pcidev),
					"id": "power",
					"text": "Power",
					"descr": "DSL power feeding mode.",
					"options": {"pwroff": "off", "pwron": "on"},
					"cssClass": "widgetManual"
				};
				c.addWidget(field, {"type": "insertBefore", "anchor": $("#advlink").parents("tr")});
			}

			/* add widgets for manual & master mode */
			if ($("#mode").val() == "master") {
				addManualMasterWidgets();
			}
		};

		/* add widgets for manual & master */
		var addManualMasterWidgets = function() {
			if ($(".widgetManualMaster").length > 0) {
				return;
			}

			/* tcpam */
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_code", pcislot, pcidev),
				"id": "tcpam",
				"text": "Coding",
				"descr": "DSL line coding.",
				"options": chipVer == "v1"	? "tcpam8 tcpam16 tcpam32"
											: "tcpam8 tcpam16 tcpam32 tcpam64 tcpam128",
				"defaultValue": "tcpam16",
				"cssClass": "widgetManualMaster",
				"onChange": function() {
					/* update rate list */
					$("#rate").setOptionsForSelect({
							"options": getRates(chipVer, $("#tcpam").val()),
							"curValue": getNearestCorrectRate(chipVer, $("#tcpam").val(), $("#rate").val())
					});
					setMrate();

					/* update description */
					$("#rate").nextAll("p").html(getDslRateDescr(chipVer, $("#tcpam").val()));
				}
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#mode").parents("tr")});

			/* rate */
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_rate", pcislot, pcidev),
				"id": "rate",
				"text": "Rate",
				"descr": getDslRateDescr(chipVer, $("#tcpam").val()),
				"onChange": setMrate,
				"cssClass": "widgetManualMaster",
				"options": getRates(chipVer, $("#tcpam").val()),
				"defaultValue": "2304"
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#tcpam").parents("tr")});
			setMrate();

			/* annex */
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_annex", pcislot, pcidev),
				"text": "Annex",
				"descr": "DSL Annex.",
				"options": {"A": "Annex A", "B": "Annex B"},
				"cssClass": "widgetManualMaster"
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#rate").parents("tr")});

			addMasterWidgets();
		};

		/* add, if not exist, widgets for EOCd mode */
		var addEocdWidgets = function() {
			if ($(".widgetEocd").length > 0) {
				return;
			}

			/* eocdMaster */
			field = {
				"type": "select",
				"name": $.sprintf("sys_eocd_chan_s%s_%s_master", pcislot, pcidev),
				"id": "eocdMaster",
				"text": "Mode",
				"descr": "DSL mode.",
				"options": {"1": "Master", "0": "Slave"},
				"cssClass": "widgetEocd",
				"onChange": onEocdMasterChange
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#ctrl").parents("tr")});

			/* profile */
			field = {
				"type": "select",
				"name": $.sprintf("sys_eocd_chan_s%s_%s_confprof", pcislot, pcidev),
				"id": "profile",
				"text": "Config profile",
				"descr": "EOC configuration profile associated with this channel.",
				"options": function() {
					/* determine compatibility for current interface */
					var compatibility = (chipVer == "v1") ? "base" : "extended";

					/* get EOC profiles */
					var profiles = config.getParsed("sys_eocd_sprof_*");

					var resultProfiles = ["default"];
					$.each(profiles, function(profileKey, profile) {
						if (compatibility == "extended" || compatibility == profile.comp) {
							resultProfiles.push(profile.name);
						}
					});

					return resultProfiles;
				}(),
				"cssClass": "widgetEocd"
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#eocdMaster").parents("tr")});

			if ($("#eocdMaster").val() == "1") {
				addEocdMasterWidgets();
			}
		};

		/* add widgets for EOCd & master */
		var addEocdMasterWidgets = function() {
			if ($(".widgetEocdMaster").length > 0) {
				return;
			}
			
			/* regs */
			field = {
				"type": "text",
				"name": $.sprintf("sys_eocd_chan_s%s_%s_regs", pcislot, pcidev),
				"text": "Regenerators",
				"descr": "Number of installed regenerators (theoretical).",
				"validator": {"min": 0},
				"cssClass": "widgetEocdMaster"
			};
			c.addWidget(field, {"type": "insertAfter", "anchor": $("#profile").parents("tr")});

			addMasterWidgets();
		};

		/* add, if not exist, widgets for master mode which are common for manual and eocd */
		var addMasterWidgets = function() {
			if ($(".widgetMaster").length > 0) {
				return;
			}

			/* pbomode */
			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_pbomode", pcislot, pcidev),
				"id": "pbomode",
				"text": "PBO forced",
				"descr": "Example: 21:13:15, STU-C-SRU1=21,SRU1-SRU2=13,...",
				"onClick": setPboval,
				"cssClass": "widgetMaster"
			};
			c.addWidget(field, {"type": "insertBefore", "anchor": $("#advlink").parents("tr")});
			setPboval();

			/* clock mode */
			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_clkmode", pcislot, pcidev),
				"id": "clockMode",
				"text": "Clock mode",
				"descr": "DSL clock mode.",
				"options": "sync plesio plesio-ref",
				"cssClass": "widgetMaster"
			};
			c.addWidget(field, {"type": "insertBefore", "anchor": $("#pbomode").parents("tr")});
		};
		
		/* add parameters which are common for all controls and modes */
		
		field = { 
			"type": "checkbox",
			"name": $.sprintf("sys_mux_%s_mxen", iface),
			"text": "Enable multiplexing",
			"descr": "Enable multiplexing on this interface.",
			"tip": "This option is equivalent to MXEN on a multiplexing page."
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_ctrl", pcislot, pcidev),
			"id": "ctrl",
			"text": "Control type",
			"descr": "Control type (manual or by EOC daemon).",
			"options": {"manual": "Manual", "eocd": "EOCd"},
			"onChange": onCtrlChange
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_advlink", pcislot, pcidev),
			"id": "advlink",
			"text": "AdvLink",
			"descr": "DSL Advanced link detection.",
			"options": "off on"
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_crc", pcislot, pcidev),
			"text": "CRC",
			"descr": "DSL CRC length.",
			"options": {"crc32": "CRC32", "crc16": "CRC16"}
		};
		c.addWidget(field);
		
		field = { 
			"type": "select",
			"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
			"text": "Fill",
			"descr": "DSL fill byte value.",
			"options": {"fill_ff": "FF", "fill_7e": "7E"}
		};
		c.addWidget(field);

		/* on submit, add or remove current channel from EOCd interfaces */
		var additionalKeys = [];
		c.addSubmit({
			"additionalKeys": additionalKeys,
			"preSubmit": function() {
				var channel = $.sprintf("%s:%s", pcislot, pcidev);

				/* eocd control */
				if ($("#ctrl").val() == "eocd") {
					/* if this channel is not yet included to the eocd channel list, include it */
					if (!config.get("sys_eocd_channels") ||
							config.get("sys_eocd_channels").search(channel) == -1) {
						$.addObjectWithProperty(additionalKeys, "sys_eocd_channels",
								config.get("sys_eocd_channels")
								? $.sprintf("%s %s", config.get("sys_eocd_channels"), channel)
								: channel);
					}
				/* manual control */
				} else {
					/* if this channel is inculded to the eocd channel list, exclude it */
					if (config.get("sys_eocd_channels") &&
							config.get("sys_eocd_channels").search(channel) != -1) {
						var channels = config.get("sys_eocd_channels").replace(channel, "");
						channels = $.trim(channels);
						$.addObjectWithProperty(additionalKeys, "sys_eocd_channels", channels);
					}
				}
			},
			"onSuccess": function() {
				updateFields($.sprintf("sys_pcicfg_s%s_%s_mrate", pcislot, pcidev), true);
			}
		});

		onCtrlChange();
	};
	
	/* show main statistics page with unit selection */
	var showStatistics = function(eocInfo) {
		var c, field;
		c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		
		/* this means that we received not JSON data */
		if (typeof eocInfo != "object") {
			c.addTitle("Error on the device while performing AJAX request or router is offline.");
			return;
		}
		
		/* if error, show corresponding field */
		if (eocInfo.eoc_error == "1") {
			c.addTitle("EOC error");
			
			field = {
				"type": "html",
				"name": "eoc_error",
				"text": "EOC ERROR",
				"descr": "EOC daemon returned error while performing this request.",
				"str": eocInfo.err_string
			};
			c.addWidget(field);
			
			return;
		}
		
		c.addTitle("Select channel unit");
		
		var onUnitChange = function() {
			/* save selected unit in cookie */
			$.cookie("unit", $("#unit").val());
			
			/* remove br and all forms, except first */
			$("form:eq(1) ~ br, form:eq(0) ~ form").remove();

			c.removeStaticMessages();
			
			if ($("#unit").val() == "general") {
				config.cmdExecute({
					"cmd": $.sprintf("%s -j -i%s", eocInfoCmd, iface),
					"callback": showGeneral,
					"dataType": "json"
				});
			} else {
				config.cmdExecute({
					"cmd": $.sprintf("%s -j -i%s -u%s", eocInfoCmd, iface, $("#unit").val()),
					"callback": showStatUnit,
					"dataType": "json"
				});
			}
		};
		
		/* create hash with channel units */
		var units = {"general": "General"};
		if (eocInfo.unit_num > 0) {
			/* statistics for each unit is available even EOC is offline */
			units['stu-c'] = "STU-C";

            /* to show STU-R we must have all regenertors and STU-C and STU-R */
            if ((parseInt(eocInfo.status.reg_num, 10) + 2) == eocInfo.unit_num) {
                units['stu-r'] = "STU-R";
            }

			for (var i = 1; i <= eocInfo.status.reg_num; i++) {
				units['sru' + i] = "SRU" + i;
			}
		}
		
		/* value of this widget is saved in cookie, because we need it between browser requests */
		field = { 
			"type": "select",
			"name": "unit",
			"cookie": true,
			"text": "Channel unit",
			"descr": "Select channel unit to view statistics.",
			"options": units,
			"onChange": onUnitChange
		};
		c.addWidget(field);
		
		page.addBr("statistics");
		
		onUnitChange();
	};
	
	/* show general interface statistics */
	var showGeneral = function(eocInfo) {
		var c, field;
		c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		
		/* this means that we received not JSON data */
		if (typeof eocInfo != "object") {
			c.addTitle("Error on the device while performing AJAX request or router is offline.");
			return;
		}
		
		/* if error, show corresponding field */
		if (eocInfo.eoc_error == "1") {
			c.addTitle("EOC error");
			
			field = {
				"type": "html",
				"name": "eoc_error",
				"text": "EOC ERROR",
				"descr": "EOC daemon returned error while performing this request.",
				"str": eocInfo.err_string
			};
			c.addWidget(field);
			
			return;
		}
	
		c.addTitle(iface + " state");
	
		field = {
			"type": "html",
			"name": "status",
			"text": "Channel link",
			"descr": "Connection of STU-C to STU-R.",
			"str": eocInfo.link == "1" ? "online": "offline"
		};
		c.addWidget(field);
		
		field = {
			"type": "html",
			"name": "regs",
			"text": "Regenerators",
			"descr": "Regenerators in channel (actual number).",
			"str": eocInfo.status.reg_num
		};
		c.addWidget(field);
		
		field = {
			"type": "html",
			"name": "pairs",
			"text": "Wire pairs",
			"descr": "Number of wire pairs in channel.",
			"str": eocInfo.loop_num
		};
		c.addWidget(field);
		
		field = {
			"type": "html",
			"name": "rate",
			"text": "Rate",
			"descr": "Channel rate value.",
			"str": eocInfo.status.rate
		};
		c.addWidget(field);
		
		field = {
			"type": "html",
			"name": "annex",
			"text": "Annex",
			"descr": "Channel annex value.",
			"str": eocInfo.status.annex
		};
		c.addWidget(field);
		
		field = {
			"type": "html",
			"name": "encoding",
			"text": "Encoding",
			"descr": "Channel tcpam value.",
			"str": eocInfo.status.tcpam
		};
		c.addWidget(field);
	};
	
	/* show unit statistics */
	var showStatUnit = function(eocInfo) {
		/* Show current state of interface */
		var showState = function(loop, side, c) {
			var field;
			var row = c.addTableRow();
			
			field = {
				"type": "html",
				"name": "state_side_" + side + loop.name,
				"str": side
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "state_loop_" + side + loop.name,
				"str": loop.name
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "snr_" + side + loop.name,
				"str": loop.cur.snr
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "lattn_" + side + loop.name,
				"str": loop.cur.lattn
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "es_" + side + loop.name,
				"str": loop.cur.es
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ses_" + side + loop.name,
				"str": loop.cur.ses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "crc_" + side + loop.name,
				"str": loop.cur.crc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "losws_" + side + loop.name,
				"str": loop.cur.losws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "uas_" + side + loop.name,
				"str": loop.cur.uas
			};
			c.addTableWidget(field, row);
		};
		
		/*
		 * Show relative counters for interface.
		 * 
		 * loop — current loop;
		 * side — current side name;
		 * c — container;
		 * unit — current unit;
		 * sideNum — index of side in sides array;
		 * loopNum — index of loop in loops array;
		 * row — row in table. If specified — it is empties, if not — new row is added to a table.
		 */
		var showRelativeCounters = function(loop, side, c, unit, sideNum, loopNum, row) {
			var field;
			
			/* empty row if it is specified */
			if (row) {
				row.empty();
			} else {
				row = c.addTableRow();
			}
			
			field = {
				"type": "html",
				"name": "tdate_" + side + loop.name,
				"str": loop.rel.date
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "ttime_" + side + loop.name,
				"str": loop.rel.time
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "relative_side_" + side + loop.name,
				"str": side
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "relative_loop_" + side + loop.name,
				"str": loop.name
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tes_" + side + loop.name,
				"str": loop.rel.es
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tses_" + side + loop.name,
				"str": loop.rel.ses
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tcrc_" + side + loop.name,
				"str": loop.rel.crc
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tlosws_" + side + loop.name,
				"str": loop.rel.losws
			};
			c.addTableWidget(field, row);
			
			field = {
				"type": "html",
				"name": "tuas_" + side + loop.name,
				"str": loop.rel.uas
			};
			c.addTableWidget(field, row);
			
			/*
			 * when user presses reset button, first AJAX request zeroes relative counters,
			 * second AJAX request updates table's row.
			 */
			field = {
				"type": "button",
				"name": "reset_" + side + loop.name,
				"text": "reset",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("%s --relative-rst -i%s -u%s -e%s", eocInfoCmd, iface, unit, side),
						"callback": function() {
							config.cmdExecute({
								"cmd": $.sprintf("%s -j -i%s -u%s", eocInfoCmd, iface, unit),
								"callback": function(eocInfo) {
									/* if error, show corresponding message */
									if (typeof eocInfo != "object" || eocInfo.eoc_error == "1") {
										row.remove();
										row = c.addTableRow();
										
										var field = {
											"type": "html",
											"name": "eoc_error",
											"str": (typeof eocInfo != "object") ?
												"Error on the device while performing AJAX request or router is offline."
												: "EOC ERROR: " + eocInfo.err_string
										};
										c.addTableWidget(field, row, 10);
										
										return;
									}
									
									showRelativeCounters(eocInfo.sides[sideNum].loops[loopNum],
											eocInfo.sides[sideNum].name, c, unit, sideNum,
											loopNum, row);
								},
								"dataType": "json"
							});
						}
					});
				}
			};
			c.addTableWidget(field, row);
		};
		
		/* show current 15 minutes and 1 day intervals */
		var showCurrentIntervals = function(loop, side, c) {
			var field;
			
			/*
			 * name — name of interval;
			 * keyName — name for key in "cur" section for current loop.
			 */
			var showInterval = function(name, keyName) {
				var row = c.addTableRow();
				
				field = {
					"type": "html",
					"name": "name_" + side + loop.name + keyName,
					"str": name
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "side_" + side + loop.name + keyName,
					"str": side
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "loop_" + side + loop.name + keyName,
					"str": loop.name
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "es_" + side + loop.name + keyName,
					"str": loop.cur[keyName].es
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "ses_" + side + loop.name + keyName,
					"str": loop.cur[keyName].ses
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "crc_" + side + loop.name + keyName,
					"str": loop.cur[keyName].crc
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "losws_" + side + loop.name + keyName,
					"str": loop.cur[keyName].losws
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "uas_" + side + loop.name + keyName,
					"str": loop.cur[keyName].uas
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "elaps_" + side + loop.name + keyName,
					"str": loop.cur[keyName].elapsed
				};
				c.addTableWidget(field, row);
			};
			
			showInterval("Curr 15 minutes", "m15int");
			showInterval("Curr 1 day", "d1int");
		};
		
		/* show all 15 minutes intervals */
		var show15MinIntervals = function(loop, side, c) {
			var field;
			
			$.each(loop.m15int, function(num, interval) {
				var row = c.addTableRow();
			
				field = {
					"type": "html",
					"name": "int_day_" + side + loop.name + interval.int,
					"str": interval.int_day
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "time_start_" + side + loop.name + interval.int,
					"str": interval.time_start
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "time_end_" + side + loop.name + interval.int,
					"str": interval.time_end
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "es_" + side + loop.name + interval.int,
					"str": interval.es
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "ses_" + side + loop.name + interval.int,
					"str": interval.ses
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "crc_" + side + loop.name + interval.int,
					"str": interval.crc
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "losws_" + side + loop.name + interval.int,
					"str": interval.losws
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "uas_" + side + loop.name + interval.int,
					"str": interval.uas
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "mon_pers_" + side + loop.name + interval.int,
					"str": interval.mon_pers
				};
				c.addTableWidget(field, row);
			});
		};
		
		/* show all 1 days intervals */
		var show1DayIntervals = function(loop, side, c) {
			var field;
			
			$.each(loop.d1int, function(num, interval) {
				var row = c.addTableRow();
				
				field = {
					"type": "html",
					"name": "time_end_" + side + loop.name + interval.int,
					"str": interval.int_day
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "es_" + side + loop.name + interval.int,
					"str": interval.es
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "ses_" + side + loop.name + interval.int,
					"str": interval.ses
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "crc_" + side + loop.name + interval.int,
					"str": interval.crc
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "losws_" + side + loop.name + interval.int,
					"str": interval.losws
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "uas_" + side + loop.name + interval.int,
					"str": interval.uas
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "mon_pers_" + side + loop.name + interval.int,
					"str": interval.mon_pers
				};
				c.addTableWidget(field, row);
			});
		};
		
		/* show sensors status */
		var showSensors = function(eocInfo, c) {
			var field;

			/* this means that we received not JSON data */
			if (typeof eocInfo != "object") {
				c.addTitle("Error on the device while performing AJAX request or router is offline.");
				return;
			}
			
			$.each(eocInfo.sensors, function(num, sensor) {
				var row = c.addTableRow();
				
				field = {
					"type": "html",
					"name": "sensor_num" + sensor.num,
					"str": sensor.num
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "sensor_state" + sensor.num,
					"str": sensor.cur
				};
				c.addTableWidget(field, row);
				
				field = {
					"type": "html",
					"name": "sensor_counter" + sensor.num,
					"str": sensor.cnt
				};
				c.addTableWidget(field, row);
			});
		};
		
		/* container */
		var c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		
		/* this means that we received not JSON data */
		if (typeof eocInfo != "object") {
			c.addTitle("Error on the device while performing AJAX request or router is offline.");
			return;
		}
		
		/* if error, show corresponding field */
		if (eocInfo.eoc_error == "1") {
			c.addTitle("EOC error");
			
			var field = {
				"type": "html",
				"name": "eoc_error",
				"text": "EOC ERROR",
				"descr": "EOC daemon returned error while performing this request.",
				"str": eocInfo.err_string
			};
			c.addWidget(field);
			
			return;
		}

		/* show message if EOC is offline and this unit is not STU-C, which is always available */
		if (eocInfo.unit != "STU-C") {
			config.cmdExecute({
				"cmd": $.sprintf("%s -j -i%s", eocInfoCmd, iface),
				"callback": function(eocInfo) {
					if (eocInfo.link == "0") {
						c.addStaticMessage("EOC is offline, this data is outdated.");
					}
				},
				"dataType": "json"
			});
		}
		
		/* add State table */
		c.addTitle(iface + " state", {"colspan": 9});
		c.addTableHeader("Side|Pair|SNR margin|LoopAttn|ES|SES|CV|LOSWS|UAS");
		
		$.each(eocInfo.sides, function(num, side) {
			$.each(side.loops, function(num, loop) {
				showState(loop, side.name, c);
			});
		});
		
		/* for regenerators add Sensors table */
		if (eocInfo.unit.search("SRU") != -1) {
			page.addBr("statistics");
			c = page.addContainer("statistics");
			c.setHelpPage("eoc");
			c.addTitle(iface + " sensors", {"colspan": 9});
			c.addTableHeader("Sensor #|Current state|Event Counter");
			
			showSensors(eocInfo, c);
		}
		
		/* add Relative counters table */
		page.addBr("statistics");
		c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		c.addTitle(iface + " relative counters", {"colspan": 10});
		c.addTableHeader("Start date|Start time|Side|Pair|ES|SES|CV|LOSWS|UAS|Reset");
		
		$.each(eocInfo.sides, function(sideNum, side) {
			$.each(side.loops, function(loopNum, loop) {
				showRelativeCounters(loop, side.name, c, eocInfo.unit, sideNum, loopNum);
			});
		});
		
		/* add Current intervals */
		page.addBr("statistics");
		c = page.addContainer("statistics");
		c.setHelpPage("eoc");
		c.addTitle(iface + " current intervals", {"colspan": 9});
		c.addTableHeader("Interval|Side|Pair|ES|SES|CV|LOSWS|UAS|Time elapsed");
		
		$.each(eocInfo.sides, function(num, side) {
			$.each(side.loops, function(num, loop) {
				showCurrentIntervals(loop, side.name, c);
			});
		});
		
		/* add 15 minutes intervals */
		$.each(eocInfo.sides, function(num, side) {
			$.each(side.loops, function(num, loop) {
				/* create table for each side and loop */
				page.addBr("statistics");
				c = page.addContainer("statistics");
				c.setHelpPage("eoc");
				c.addTitle(
					$.sprintf("%s %s %s 15 Minutes error intervals", iface, side.name, loop.name),
					{"colspan": 9});
				c.addTableHeader("Date|Start time|End time|ES|SES|CV|LOSWS|UAS|Monitoring (%)");
				show15MinIntervals(loop, side.name, c);
			});
		});
		
		/* add 1 days intervals */
		$.each(eocInfo.sides, function(num, side) {
			$.each(side.loops, function(num, loop) {
				/* create table for each side and loop */
				page.addBr("statistics");
				c = page.addContainer("statistics");
				c.setHelpPage("eoc");
				c.addTitle(
					$.sprintf("%s %s %s 1 Day error intervals", iface, side.name, loop.name),
					{"colspan": 7});
				c.addTableHeader("Date|ES|SES|CV|LOSWS|UAS|Monitoring (%)");
				show1DayIntervals(loop, side.name, c);
			});
		});
	};

	/* manage EOC profiles */
	var eocProfiles = function() {
		var c, field;
		c = page.addContainer("eoc_profiles");
		c.setSubsystem("eoc_profile");
		c.setHelpPage("dsl-eoc");

		/*
		 * Callback for generateList(), returns correct value for rate, if it's value is -1.
		 *
		 * data - hash with current key's variable info.
		 */
		var showCorrectRate = function(data) {
			if (data.varName == "rate" && data.varValue == "-1") {
				return data.keyValues.mrate;
			} else {
				return data.varValue;
			}
		};

		/* get available TCPAM values depending on compatibility level */
		var getTcpamValues = function() {
			return $("#comp").val() == "base"
					? "tcpam8 tcpam16 tcpam32"
					: "tcpam8 tcpam16 tcpam32 tcpam64 tcpam128";
		};

		/* add widgets which values are calculated in runtime */
		var addProfileWidgets = function(list) {
			var field;

			/* add or remove mrate field depending on rate value */
			var setMrate = function() {
				/* if rate is "other", add text widget to enter rate value */
				if ($("#rate").val() == "-1" && $("#mrate").length == 0) {
					var field = {
						"type": "text",
						"name": "mrate",
						"validator": {"required": true, "min": 0},
						"onChange": function() {
							$("#mrate").val(getNearestCorrectRate(
									($("#comp").val() == "base") ? "v1" : "v2",
									$("#tcpam").val(), $("#mrate").val()));
						},
						"valueFilter": function(value) {
							return getNearestCorrectRate(($("#comp").val() == "base") ? "v1" : "v2",
									$("#tcpam").val(), value);
						}
					};
					list.addDynamicSubWidget(field, {"type": "insertAfter", "anchor": "#rate"});
				/* otherwise, remove it */
				} else if ($("#rate").val() != "-1") {
					$("#mrate").remove();
				}
			};

			/* name */
			field = {
				"type": "text",
				"name": "name",
				"text": "Name",
				"descr": "Name of profile.",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addDynamicWidget(field);

			/* comp */
			field = {
				"type": "select",
				"name": "comp",
				"text": "Compatibility",
				"descr": "Profile's compatibility level.",
				"options": "base extended",
				"onChange": function() {
					var comp = ($("#comp").val() == "base") ? "v1" : "v2";

					/* update tcpam list */
					$("#tcpam").setOptionsForSelect({
							"options": getTcpamValues(),
							"curValue": $("#tcpam").val(),
							"defaultValue": "tcpam16"
					});

					/* update rate list */
					$("#rate").setOptionsForSelect({
							"options": getRates(comp, $("#tcpam").val()),
							"curValue": getNearestCorrectRate(comp, $("#tcpam").val(), $("#rate").val())
					});
					setMrate();

					/* update description */
					$("#rate").nextAll("p").html(getDslRateDescr(comp, $("#tcpam").val()));
				}
			};
			list.addDynamicWidget(field);

			/* tcpam */
			field = {
				"type": "select",
				"name": "tcpam",
				"text": "Encoding",
				"descr": "DSL line coding.",
				"options": getTcpamValues(),
				"defaultValue": "tcpam16",
				"onChange": function() {
					var comp = ($("#comp").val() == "base") ? "v1" : "v2";

					/* update rate list */
					$("#rate").setOptionsForSelect({
							"options": getRates(comp, $("#tcpam").val()),
							"curValue": getNearestCorrectRate(comp, $("#tcpam").val(), $("#rate").val())
					});
					setMrate();

					/* update description */
					$("#rate").nextAll("p").html(getDslRateDescr(comp, $("#tcpam").val()));
				}
			};
			list.addDynamicWidget(field);

			/* rate */
			field = {
				"type": "select",
				"name": "rate",
				"text": "Rate",
				"descr": getDslRateDescr(($("#comp").val() == "base") ? "v1" : "v2", $("#tcpam").val()),
				"options": getRates(($("#comp").val() == "base") ? "v1" : "v2", $("#tcpam").val()),
				"defaultValue": "2304",
				"onChange": setMrate
			};
			list.addDynamicWidget(field);
			setMrate();

			/* annex */
			field = {
				"type": "select",
				"name": "annex",
				"text": "Annex",
				"descr": "DSL Annex.",
				"options": {"A": "Annex A", "B": "Annex B"}
			};
			list.addDynamicWidget(field);

			/* pwron */
			field = {
				"type": "select",
				"name": "power",
				"text": "Power",
				"descr": "DSL power feeding mode.",
				"options": "off on"
			};
			list.addDynamicWidget(field);
		};

		/* on profile editing, forbid changing it's name and compatibility */
		var onEditProfile = function(list) {
			$("#name").attr("readonly", true);
			$("#comp").setSelectReadonly(true);
		};

		/* check that this profile is not in use, if it is in use - alert user */
		var checkOnProfileDelete = function(item) {
			/* get deleting profile values */
			var profile = config.getParsed(item);

			/* get profiles, which are in use */
			var channels = config.getByRegexp(/(sys_eocd_chan_)*(_confprof)/);

			/* list of interfaces, which are using this profile */
			var profileIfaces = "";

			/* KDB keys which contain this profile (e.g., sys_eocd_chan_s0004_0_confprof) */
			var profileIfacesKeys = [];

			/* go through list of profiles, which are in use */
			$.each(channels, function(key, value) {
				/* if this profile is in use */
				if (value == profile.name) {
					profileIfacesKeys.push(key);
					
					/* get pcislot and pcidev of interface, which is using this profile */
					var pciSlotPciDev = key.replace("sys_eocd_chan_s", "").replace("_confprof", "")
							.split("_");

					/* if pcislot and pcidev are available */
					if (pciSlotPciDev.constructor == Array && pciSlotPciDev.length >= 2) {
						/* get ifaces for this pcislot */
						var ifaces = config.getParsed(
								$.sprintf("sys_pcitbl_s%s_ifaces", pciSlotPciDev[0]));

						/* if ifaces is available */
						if (ifaces.constructor == Array && ifaces.length > 0) {
							/* if interface with this pcislot and pcidev is available */
							if (ifaces[pciSlotPciDev[1]]) {
								/* add it to the list of interfaces, which are using this profile */
								profileIfaces += ifaces[pciSlotPciDev[1]] + " ";
							}
						}
					}

					/* if we didn't find interfaces name */
					if (profileIfaces.length == 0) {
						profileIfaces = "undefined";
					}
				}
			});
			profileIfaces = $.trim(profileIfaces);

			if (profileIfaces.length > 0) {
				return	{
							"deleteAllowed": true,
							"message": $.sprintf("%s %s. %s", _("This profile is used by interfaces:"),
									profileIfaces, _("If this profile will be deleted, it will be replaced by default profile. Are you sure you want to delete this profile?")),
							"actionOnDelete": function() {
								/* set profile to default on interfaces, which are using this profile */
								var kdbFields = [];
								$.each(profileIfacesKeys, function(num, profileIfacesKey) {
									$.addObjectWithProperty(kdbFields, profileIfacesKey, "default");
								});
								config.kdbSubmit(kdbFields);
							}
						};
			} else {
				return {"deleteAllowed": true};
			}
		};

		/* check that name of this profile is unique */
		var checkOnProfileAdd = function(isAdding) {
			/* allow editing */
			if (!isAdding) {
				return {"addAllowed": true};
			}
			
			var newProfileName = $("#name").val();
			var profiles = config.getParsed("sys_eocd_sprof_*");
			var uniqueProfile = true;

			$.each(profiles, function(key, profile) {
				if (newProfileName == profile.name) {
					uniqueProfile = false;
					return false;
				}
			});

			if (uniqueProfile) {
				return	{"addAllowed": true};
			} else {
				return	{
							"addAllowed": false,
							"message": _("Profile with such name is already exists.")
						};
			}
		};

		var list = c.createList({
			"tabId": "eoc_profiles",
			"header": ["Name", "Compatibility", "Encoding", "Rate", "Annex", "Power"],
			"varList": ["name", "comp", "tcpam", "rate", "annex", "power"],
			"listItem": "sys_eocd_sprof_",
			"addMessage": "Add EOC profile",
			"editMessage": "Edit EOC profile",
			"listTitle": "EOC profiles",
			"onAddOrEditItemRender": addProfileWidgets,
			"onEditItemRender": onEditProfile,
			"processValueFunc": showCorrectRate,
			"checkOnDelete": checkOnProfileDelete,
			"checkOnSubmit": checkOnProfileAdd
		});

		list.generateList();
	};

	/* get driver name to determine interface version (MR16H/MR17H) */
	var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));

	/* status tab available for both MR16H/MR17H */
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Status();
			} else if (type == config.getOEM("MR17H_DRVNAME")) {
				var c = page.addContainer("status");

				/* check that router is online */
				if (!config.isOnline()) {
					c.addTitle("Router is offline");
					return;
				}

				c.addTitle("Loading data...");
				config.cmdExecute({
					"cmd": "./dsl17status_json.sh " + iface,
					"callback": sg17Status,
					"dataType": "json"
				});
			}
		}
	});

	/* settings tab available for both MR16H/MR17H */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			if (type == config.getOEM("MR16H_DRVNAME")) {
				sg16Settings();
			} else if (type == config.getOEM("MR17H_DRVNAME")) {
				sg17Settings();
			}
		}
	});

	/* statistics and EOC profiles tab available only for MR17H */
	if (type == config.getOEM("MR17H_DRVNAME")) {
		/* path for eoc-info utility */
		var eocInfoCmd = "/sbin/eoc-info";

		/* config path */
		var confPath = $.sprintf("%s/%s/sg_private", config.getOEM("sg17_cfg_path"), iface);

		/* power status */
		var pwrPresence = config.getCachedOutput($.sprintf("/bin/cat %s/pwr_source", confPath));

		/* chip version */
		var chipVer = config.getCachedOutput($.sprintf("/bin/cat %s/chipver", confPath));

		/* list of rates */
		var rateList8 = [192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840];
		var rateList16 = [192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840];
		var rateList32_v1 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696];
		var rateList32_v2 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10176];
		var rateList64 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12736];
		var rateList128 = [768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12800,14080];

		/* return rate list depending on passed chipVer and tcpam */
		var getRateList = function(chipVer, tcpam) {
			/* v1 */
			if (chipVer == "v1") {
				switch (tcpam) {
				case "tcpam8":
					return rateList8;
				case "tcpam16":
					return rateList16;
				case "tcpam32":
					return rateList32_v1;
				}
			/* v2 */
			} else {
				switch (tcpam) {
				case "tcpam8":
					return rateList8;
				case "tcpam16":
					return rateList16;
				case "tcpam32":
					return rateList32_v2;
				case "tcpam64":
					return rateList64;
				case "tcpam128":
					return rateList128;
				}
			}
		};

		/* return minimum rate for passed rateList */
		var getMinRate = function(rateList) {
			return rateList[0];
		};

		/* return maximum rate for passed rateList */
		var getMaxRate = function(rateList) {
			return rateList[rateList.length - 1];
		};

		/* get rate list depending on TCPAM and chipVer value, with added "other" value */
		var getRates = function(chipVer, tcpam) {
			var rates = getRateList(chipVer, tcpam);
			var ratesList = {};

			$.each(rates, function(num, rate) {
				ratesList[rate] = rate;
			});
			ratesList['-1'] = "other";

			return ratesList;
		};

		/* returns correct rate for passed chipVer and tcpam */
		var getNearestCorrectRate = function(chipVer, tcpam, rate) {
			var rateList = getRateList(chipVer, tcpam);
			var availableRate = parseInt(rate, 10);
			var min = getMinRate(rateList);
			var max = getMaxRate(rateList);

			if (availableRate < min || availableRate < 64) {
				return min;
			} else if (availableRate > max) {
				return max;
			} else if ((availableRate % 64) != 0) {
				var remainder = availableRate % 64;
				if (remainder < 32) {
					availableRate -= remainder;
					return availableRate;
				} else {
					availableRate += (64 - remainder);
					return availableRate;
				}
			} else {
				return availableRate;
			}
		};

		/* return description for Rate field */
		var getDslRateDescr = function(chipVer, tcpam) {
			var rateList = getRateList(chipVer, tcpam);
			return $.sprintf("%s, %s %s %s %s.", _("DSL line rate in kbit/s"), _("from"),
					getMinRate(rateList), _("to"), getMaxRate(rateList));
		};

		page.addTab({
			"id": "eoc_profiles",
			"name": "EOC profiles",
			"func": eocProfiles
		});

		page.addTab({
			"id": "statistics",
			"name": "Statistics",
			"func": function() {
				var c;

				/* do not show statistics for manual-controlled interfaces */
				if (config.get($.sprintf("sys_pcicfg_s%s_%s_ctrl", pcislot, pcidev)) == "manual") {
					c = page.addContainer("statistics");
					c.addTitle("Statistics is available only for interfaces with EOCd control");
					return;
				} else if (config.get($.sprintf("sys_eocd_chan_s%s_%s_master", pcislot, pcidev)) != "1") {
					c = page.addContainer("statistics");
					c.addTitle("Statistics is available only for master interface");
					return;
				} else if (!config.isOnline()) {
					c = page.addContainer("statistics");
					c.addTitle("Router is offline");
					return;
				}

				config.cmdExecute({
					"cmd": $.sprintf("%s -j -i%s", eocInfoCmd, iface),
					"callback": showStatistics,
					"dataType": "json"
				});
			}
		});
	}

	page.generateTabs();
};
