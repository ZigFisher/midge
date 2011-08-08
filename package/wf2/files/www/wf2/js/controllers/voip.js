Controllers.voipSettings = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.setSubsystem("svd-main");
			c.addTitle("VoIP settings");
			
			/* General settings */
			c.addTitle("General settings", {"internal": true, "help": {"page": "voip.settings"}});
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_first",
				"text": "RTP port start",
				"descr": "Begin of ports range to use for RTP.",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_settings_rtp_port_last",
				"text": "RTP port end",
				"descr": "End of ports range to use for RTP.",
				"validator": {"required": true, "min": 0, "max": 65535}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_settings_log",
				"text": "Logging level",
				"descr": "Level of logging.",
				"options": function() {
					var options = {};
					
					options["-1"] = "off";
					for (var i = 0; i < 10; i++) {
						options[i] = i;
					}
					
					return options;
				}()
			};
			c.addWidget(field);

            /* ToS settings */
			c.addTitle("ToS settings", {"internal": true});

			field = {
				"type": "text",
				"name": "sys_voip_settings_rtp_tos",
				"text": "RTP ToS",
				"descr": "ToS (8 bits) for RTP packets.",
				"validator": {"required": true, "tos": true}
			};
			c.addWidget(field);

            field = {
				"type": "text",
				"name": "sys_voip_settings_sip_tos",
				"text": "SIP ToS",
				"descr": "ToS (8 bits) for SIP packets.",
				"validator": {"required": true, "tos": true}
			};
			c.addWidget(field);
			
			/* SIP settings */
			c.addTitle("SIP settings", {"internal": true, "help": {"page": "voip.sip"}});
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_registrar",
				"text": "Registrar",
				"descr": "SIP registrar to register on.",
				"tip": "e.g., <i>sip:server</i>",
				"validator": {"voipRegistrar": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_username",
				"text": "Username",
				"descr": "Username on SIP registrar.",
				"tip": "e.g., <i>user</i>"
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "sys_voip_sip_password",
				"text": "Password",
				"descr": "Password on SIP registrar."
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_voip_sip_user_sip_uri",
				"text": "User SIP URI",
				"tip": "e.g., <i>sip:user@server</i>",
				"validator": {"voipSipUri": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "select",
				"name": "sys_voip_sip_chan",
				"text": "FXS channel",
				"descr": "FXS channel for incoming SIP-calls.",
				"options": function() {
					/* create array with FSX ports */
					var fxsChannels = [];
					var channels = config.getCachedOutput("voipChannels");
					
					if (channels) {
						$.each(channels.split("\n"), function(num, record) {
							if (record.length == 0) return true;
							
							/* channel[0] — number of channel, channel[1] — type of channel */
							var channel = record.split(":");
							if (channel[1] == "FXS") {
                                fxsChannels.push(channel[0]);
                            }
						});
					}
					
					return fxsChannels;
				}()
			};
			c.addWidget(field);
			
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};

/* Hotline, FXO, FXS */
Controllers.voipHotline = function() {
	var page = this.Page();

	page.addTab({
		"id": "hotline",
		"name": "Hotline",
		"func": function() {
			var c, field;
			c = page.addContainer("hotline");
			c.setSubsystem("svd-hotline");
			c.setHelpPage("voip.hotline");
			c.addTitle("Hotline settings", {"colspan": 5});

			c.addTableHeader("Channel|Type|Hotline|Complete number|Comment");
			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }

                /* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

                /* VF channels are not supported */
                if (channel[1] == "VF") {
                    return true;
                }

				var row = c.addTableRow();

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

				field = {
					"type": "html",
					"name": channel[1] + channel[0],
					"str": channel[1]
				};
				c.addTableWidget(field, row);

				field = {
					"type": "checkbox",
					"name": $.sprintf("sys_voip_hotline_%s_hotline", channel[0]),
					"id": $.sprintf("sys_voip_hotline_%s_hotline", channel[0]),
					"tip": "Enable hotline for this channel."
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_hotline_%s_number", channel[0]),
					"tip": "Number to call on channel event.",
					"validator":
						{
							"required": $.sprintf("#sys_voip_hotline_%s_hotline:checked", channel[0]),
							"voipCompleteNumber": true
						}
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_hotline_%s_comment", channel[0]),
					"validator": {"alphanumU": true}
				};
				c.addTableWidget(field, row);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

/* VF */
Controllers.voipVF = function() {
	var page = this.Page();

	page.addTab({
		"id": "vf",
		"name": "Voice frequency channels",
		"func": function() {
			var c = page.addContainer("vf");
			c.setSubsystem("svd-vf");

            var colNum = 8;
			c.addTitle("Voice frequency channels", {"colspan": colNum});

            /*
             * Parameters for codecs:
             * - pkt_sz — default value;
             * - pkt_sz_ro — read-only (default set to false);
             * - pkt_sz_vals — available values (by default "2.5 5 5.5 10 11 20 30 40 50 60");
             * - payload — default value;
             * - bitpack — default value;
             * - bitpack_ro — read-only (default set to false);
             */
            var codecsParameters = {
                "aLaw": {"pkt_sz": "20", "payload": "08", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
				"g729": {"pkt_sz": "10", "payload": "18", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "10 20 30 40 60"},
				"g723": {"pkt_sz": "30", "payload": "4", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "30 60"},
				"iLBC_133": {"pkt_sz": "30", "payload": "100", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_ro": true},
				"g729e": {"pkt_sz": "10", "payload": "101", "bitpack": "rtp", "bitpack_ro": true, "pkt_sz_vals": "10 20 30 40 60"},
				"g726_16": {"pkt_sz": "10", "payload": "102", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
				"g726_24": {"pkt_sz": "10", "payload": "103", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
				"g726_32": {"pkt_sz": "10", "payload": "104", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"},
				"g726_40": {"pkt_sz": "10", "payload": "105", "bitpack": "aal2", "pkt_sz_vals": "5 5.5 10 11 20 30 40 50 60"}
            };

            var pktszDefaultValues = "2.5 5 5.5 10 11 20 30 40 50 60";

            /* set codec parameters */
            var onCodecChange = function(channel) {
                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel)).val();

                /* set values for pkt_sz */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel)).setOptionsForSelect({
                        "options": codecsParameters[codec].pkt_sz_vals == undefined
                                ? pktszDefaultValues
                                : codecsParameters[codec].pkt_sz_vals
                });

                /* set pkt_sz default */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel))
                        .val(codecsParameters[codec].pkt_sz);

                /* set/unset pkt_sz read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel))
                        .setSelectReadonly(codecsParameters[codec].pkt_sz_ro == undefined
                                ? false
                                : codecsParameters[codec].pkt_sz_ro);

                /* set payload default */
                $($.sprintf("#sys_voip_vf_channels_%s_payload", channel))
                        .val(codecsParameters[codec].payload);

                /* set bitpack default */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel))
                        .val(codecsParameters[codec].bitpack);

                /* set/unset bitpack read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel))
                        .setSelectReadonly(codecsParameters[codec].bitpack_ro == undefined
                                ? false
                                : codecsParameters[codec].bitpack_ro);
            };

			c.addTableHeader("Chan|EN|Router ID|R. chan|Codec|Packet. time|Payload|Bitpack");
            c.addTableTfootStr("Chan - local channel.", colNum);
            c.addTableTfootStr("EN - enable channel.", colNum);
            c.addTableTfootStr("Router ID - ID of a router to connect with.", colNum);
            c.addTableTfootStr("R. chan - VF channel on the remote router.", colNum);
            c.addTableTfootStr("Codec - codec to use.", colNum);
            c.addTableTfootStr("Packet. time - packetization time in ms.", colNum);
            c.addTableTfootStr("Payload - RTP codec identificator.", colNum);
            c.addTableTfootStr("Bitpack - bits packetization type.", colNum);

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

                /* only VF channels */
                if (channel[1] != "VF") {
                    return true;
                }

                /* local channel */
				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

                /* enabled */
                field = {
					"type": "checkbox",
					"name": $.sprintf("sys_voip_vf_channels_%s_enabled", channel[0])
				};
				c.addTableWidget(field, row);

                /* pair_route */
                field = {
					"type": "text",
					"name": $.sprintf("sys_voip_vf_channels_%s_pair_route", channel[0]),
					"validator": {
                            "required": $.sprintf("#sys_voip_vf_channels_%s_enabled:checked", channel[0]),
                            "voipRouterIDWithSelf": true
                    }
				};
				c.addTableWidget(field, row);

                /* pair_chan */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_vf_channels_%s_pair_chan", channel[0]),
					"options": function() {
                        var remoteChannels = [];
                        for (var i = 0; i < 32; i++) {
                            remoteChannels.push(((i < 10) ? "0" + i : i));
                        }
                        return remoteChannels;
                    }()
				};
				c.addTableWidget(field, row);

                /* codec */
                field = {
					"type": "select",
					"name": $.sprintf("sys_voip_vf_channels_%s_codec", channel[0]),
					"options": function() {
                        var codecs = [];
                        $.each(codecsParameters, function(key) {
                            codecs.push(key);
                        });
                        return codecs;
                    }(),
                    "onChange": function() {
                        onCodecChange(channel[0]);
                    }
				};
				c.addTableWidget(field, row);

                var codec = $($.sprintf("#sys_voip_vf_channels_%s_codec", channel[0])).val();

                /* pkt_sz */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_vf_channels_%s_pkt_sz", channel[0]),
                    "options": codecsParameters[codec].pkt_sz_vals == undefined
                            ? pktszDefaultValues
                            : codecsParameters[codec].pkt_sz_vals,
                    "defaultValue": codecsParameters[codec].pkt_sz
				};
				c.addTableWidget(field, row);

                /* set/unset pkt_sz read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_pkt_sz", channel[0]))
                        .setSelectReadonly(codecsParameters[codec].pkt_sz_ro == undefined
                                ? false
                                : codecsParameters[codec].pkt_sz_ro);

				/* payload */
                field = {
                    "type": "text",
                    "name": $.sprintf("sys_voip_vf_channels_%s_payload", channel[0]),
                    "defaultValue": codecsParameters[codec].payload,
                    "validator": {
                            "required": $.sprintf("#sys_voip_vf_channels_%s_enabled:checked", channel[0]),
                            "voipPayload": true
                    }
                };
                c.addTableWidget(field, row);

                /* bitpack */
                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_channels_%s_bitpack", channel[0]),
                    "options": "rtp aal2",
                    "defaultValue": codecsParameters[codec].bitpack
                };
                c.addTableWidget(field, row);

                /* set/unset bitpack read-only */
                $($.sprintf("#sys_voip_vf_channels_%s_bitpack", channel[0]))
                        .setSelectReadonly(codecsParameters[codec].bitpack_ro == undefined
                                ? false
                                : codecsParameters[codec].bitpack_ro);
			});

			c.addSubmit();
		}
	});

    page.addTab({
		"id": "vf_settings",
		"name": "Settings",
		"func": function() {
			var c = page.addContainer("vf_settings");
			c.setSubsystem("svd-vf_settings");
			c.addTitle("Settings", {"colspan": 3});

			c.addTableHeader("Channel|Wires|Transmit type");
            c.addTableTfootStr("4-Wire Normal: Tx (In) = -13 dBr, Rx (Out) = +4 dBr", 3);
            c.addTableTfootStr("4-Wire Transit: Tx (In) = +4 dBr, Rx (Out) = +4 dBr", 3);
            c.addTableTfootStr("2-Wire Normal: Tx (In) = 0 dBr, Rx (Out) = -7 dBr", 3);
            c.addTableTfootStr("2-Wire Transit: Tx (In) = -3.5 dBr, Rx (Out) = -3.5 dBr", 3);

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

                /* only VF channels */
                if (channel[1] != "VF") {
                    return true;
                }

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

				field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_settings_%s_wire_type", channel[0]),
					"options": {"2w": "2-wire", "4w": "4-wire"},
                    "defaultValue": "4w"
				};
                c.addTableWidget(field, row);

                field = {
                    "type": "select",
                    "name": $.sprintf("sys_voip_vf_settings_%s_transmit_type", channel[0]),
					"options": "normal transit"
				};
				c.addTableWidget(field, row);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

/* Routes */
Controllers.voipRoutes = function() {
	var page = this.Page();

	page.addTab({
		"id": "voipRoute",
		"name": "Routes",
		"func": function() {
			var c, field;
			c = page.addContainer("voipRoute");
			c.setSubsystem("svd-routet");
			c.setHelpPage("voip.route");

			var list = c.createList({
				"tabId": "voipRoute",
				"header": ["Router ID", "IP-address", "Comment"],
				"varList": ["router_id", "address", "comment"],
				"listItem": "sys_voip_route_",
				"addMessage": "Add VoIP route",
				"editMessage": "Edit VoIP route",
				"listTitle": "VoIP route table",
				"helpPage": "voip.route",
				"helpSection": "voip.route.add"
			});

			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable rule",
				"defaultState": "checked"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "router_id",
				"text": "Router ID",
				"descr": "Router ID",
				"validator": {"required": true, "voipRouterID": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "address",
				"text": "Address",
				"descr": "Router address",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "comment",
				"text": "Comment",
				"descr": "Comment for this record"
			};
			list.addWidget(field);

			list.generateList();
		}
	});

	page.generateTabs();
};

/* Phone book */
Controllers.voipPhoneBook = function() {
	var page = this.Page();

	page.addTab({
		"id": "address",
		"name": "Phone book",
		"func": function() {
			var c, field;
			c = page.addContainer("address");
			c.setSubsystem("svd-addressb");
			c.setHelpPage("voip.address");

			var list = c.createList({
				"tabId": "address",
				"header": ["Short number", "Complete number", "Comment"],
				"varList": ["short_number", "complete_number", "comment"],
				"listItem": "sys_voip_address_",
				"addMessage": "Add record",
				"editMessage": "Edit record",
				"listTitle": "Phone book",
				"helpPage": "voip.address",
				"helpSection": "voip.address.add"
			});

			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable rule.",
				"defaultState": "checked"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "short_number",
				"text": "Short number",
				"descr": "Short number for speed dialing.",
				"validator": {"required": true, "voipShortNumber": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "complete_number",
				"text": "Complete number",
				"descr": "Complete telephone number.",
				"tip": "Enter phone number in format: router_id-router_channel-optional_number (e.g., 300-02 or 300-02-3345), " +
					"or SIP address in format: #sip:sip_uri# (e.g., #sip:user@domain#)",
				"validator": {"required": true, "voipCompleteNumber": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "comment",
				"text": "Comment",
				"descr": "Comment for this record."
			};
			list.addWidget(field);

			list.generateList();
		}
	});

	page.generateTabs();
};

/* Audio, FXO, FXS, VF */
Controllers.voipAudio = function() {
	var page = this.Page();

	page.addTab({
		"id": "rtp",
		"name": "Audio",
		"func": function() {
			var c = page.addContainer("rtp");
            var colNum = 8;
			c.setSubsystem("svd-rtp");
			c.addTitle("Audio settings", {"colspan": colNum});

			c.addTableHeader("Channel|Type|Tx.A|Rx.A|Tx.C|Rx.C|VAD|HPF");
            c.addTableTfootStr("Tx.A: Transmit volume settings for Analog module (outcome volume level).", colNum);
            c.addTableTfootStr("Rx.A: Receive volume settings for Analog module (income volume level).", colNum);
            c.addTableTfootStr("Tx.C: Transmit volume settings for Coder module (outcome volume level).", colNum);
            c.addTableTfootStr("Rx.C: Receive volume settings for Coder module (income volume level).", colNum);
            c.addTableTfootStr("VAD:", colNum);
            c.addTableTfootStr(" - On: voice activity detection on, in this case also comfort noise and spectral information (nicer noise) is switched on.", colNum);
            c.addTableTfootStr(" - Off: no voice activity detection.", colNum);
            c.addTableTfootStr(" - G711: voice activity detection on with comfort noise generation without spectral information.", colNum);
            c.addTableTfootStr(" - CNG_only: voice activity detection on with comfort noise generation without silence compression.", colNum);
            c.addTableTfootStr(" - SC_only: voice activity detection on with silence compression without comfort noise generation.", colNum);
            c.addTableTfootStr("HPF: income high-pass filter.", colNum);

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

                field = {
					"type": "html",
					"name": channel[0] + "_type",
					"str": channel[1]
				};
				c.addTableWidget(field, row);

				/* calculate volume values */
				var vol = "";
				for (var i = -24; i <= 24; i += 1) {
					vol += i + " ";
				}
				vol = $.trim(vol);

                /* Tx.A */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_txa", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);

				/* Rx.A */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_rxa", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);

				/* Tx.C */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_cod_tx_vol", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);

				/* Rx.C */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_cod_rx_vol", channel[0]),
					"options": vol,
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);

				/* VAD */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_vad", channel[0]),
					"options": "off on g711 CNG_only SC_only",
					"defaultValue": "off"
				};
				c.addTableWidget(field, row);

				/* HPF */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_sound_%s_hpf", channel[0]),
					"options": {"0": "off", "1": "on"},
					"defaultValue": "0"
				};
				c.addTableWidget(field, row);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

/* Codecs */
Controllers.voipCodecs = function() {
	var page = this.Page();

	page.addTab({
		"id": "codecs",
		"name": "Codecs",
		"func": function() {
			var c = page.addContainer("codecs");
			c.setSubsystem("svd-quality");
			c.addTitle("Codecs settings", {"colspan": 5});

			/* default values */
			var pktszDefault = {
				"aLaw": "20",
				"g729": "10",
				"g723": "30",
				"iLBC_133": "30",
				"g729e": "10",
				"g726_16": "10",
				"g726_24": "10",
				"g726_32": "10",
				"g726_40": "10",
				"none": ""
			};

			var payloadDefault = {
				"aLaw": "08",
				"g729": "18",
				"g723": "4",
				"iLBC_133": "100",
				"g729e": "101",
				"g726_16": "102",
				"g726_24": "103",
				"g726_32": "104",
				"g726_40": "105",
				"none": ""
			};

			var codecs = ["g729", "aLaw", "g723", "iLBC_133",
				"g729e", "g726_16", "g726_24", "g726_32", "g726_40", "none"];

			/*
			 * At one time only one type of codec can be selected.
             * If current codec type is none, set next to none too and disable all next.
			 *
			 * scope — settings scope;
			 * i — codec index.
			 */
			var setUniqueType = function(scope, i) {
				setDefaults(scope, i);
				var newVal = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
				if (newVal != "none") {
					$(".type_" + scope).not($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i))
                            .each(function(num, element) {
						if ($(element).val() == newVal) {
                            $(element).val("none");
                        }
					});
				}

				var allNone = true;
                var isNone = false;
				$(".type_" + scope).each(function(num, element) {
					/* set type of current codec to none, if we've set isNone to true in previous step */
                    if (isNone) {
						$(element).val("none").attr("readonly", true).attr("disabled", true);
                        setDefaults(scope, num);
                    /* if type of current codec is none, next will be set to none too */
                    } else if ($(element).val() == "none") {
                        isNone = true;
                        $(element).removeAttr("readonly").removeAttr("disabled");
					}

                    if ($(element).val() != "none") {
						allNone = false;
					}
				});

                /* if all types is "none", set first to aLaw. */
				if (allNone) {
                    $(".type_" + scope).eq(0).val("aLaw");
                    setDefaults(scope, 0);

                    $(".type_" + scope).eq(1).removeAttr("readonly").removeAttr("disabled");
                }
			};

			/*
			 * Set default values depending on selected code type.
			 *
			 * scope — settings scope;
			 * i — codec index.
			 */
			var setDefaults = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();

				$($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i)).val(pktszDefault[codec]);
				$($.sprintf("#sys_voip_quality_%s_codec%s_payload", scope, i)).val(payloadDefault[codec]);

				/* set default for bitpack */
				if (codec.search("g726") != -1) {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i)).val("aal2");
				} else {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i)).val("rtp");
				}

				setPtksz(scope, i);
				setBitpackReadonly(scope, i);
			};

			/*
			 * Set for some codecs bitpack readonly.
			 */
			var setBitpackReadonly = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();

				if (codec.search("g726") != -1) {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i))
						.removeAttr("readonly").removeAttr("disabled");
				} else {
					$($.sprintf("#sys_voip_quality_%s_codec%s_bitpack", scope, i))
						.attr("readonly", true).attr("disabled", true);
				}
			};

			/*
			 * Set for some codecs pkt_sz readonly.
			 */
			var setPtksz = function(scope, i) {
				var codec = $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, i)).val();
                var pktszField = $($.sprintf("#sys_voip_quality_%s_codec%s_pktsz", scope, i));

                /* set default pkt_sz values */
                pktszField.setOptionsForSelect({
                        "options": "2.5 5 5.5 10 11 20 30 40 50 60",
                        "curValue": pktszField.val()
                });

				/* for iLBC_133 pkt_sz is read-only */
				if (codec == "iLBC_133") {
					/* set disabled and readonly attributes */
					pktszField.attr("readonly", true).attr("disabled", true);
				/* for g723 set fixed values: 30 and 60 */
                } else if (codec == "g723") {
                    pktszField.setOptionsForSelect({
                            "options": "30 60",
                            "curValue": pktszField.val()
                    });

                    /* remove disabled and readonly attributes */
					pktszField.removeAttr("readonly").removeAttr("disabled");
                } else if (codec == "aLaw" || codec.search("g726") != -1) {
                    pktszField.setOptionsForSelect({
                            "options": "5 5.5 10 11 20 30 40 50 60",
                            "curValue": pktszField.val()
                    });

                    /* remove disabled and readonly attributes */
					pktszField.removeAttr("readonly").removeAttr("disabled");
                } else if (codec == "g729" || codec == "g729e") {
                    pktszField.setOptionsForSelect({
                            "options": "10 20 30 40 60",
                            "curValue": pktszField.val()
                    });

                    /* remove disabled and readonly attributes */
					pktszField.removeAttr("readonly").removeAttr("disabled");
                } else {
					/* remove disabled and readonly attributes */
					pktszField.removeAttr("readonly").removeAttr("disabled");
				}
			};

			/*
			 * Add specified number of widgets for specified scope.
			 *
			 * num — number of widgets;
			 * scope — scope (external, internal).
			 */
			var addCodecsWidgets = function(num, scope) {
				for (var i = 0; i < num; i++) {
					var field;
					var row = c.addTableRow();

					/* priority */
					field = {
						"type": "html",
						"name": "priority_" + scope + i,
						"str": "" + i
					};
					c.addTableWidget(field, row);

					/* type */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_type", scope, i),
						"options": codecs,
						"cssClass": "type_" + scope,
						/*
						 * We use double-closure here, because in single-closure each
						 * onChange callback will have var i with value of 3. More info
						 * in http://dklab.ru/chicken/nablas/39.html.
						 */
						"onChange": function(x) {
							return function() {
								setUniqueType(scope, x);
							}
						}(i)
					};
					if (num > 2) {
                        field.defaultValue = "none";
                    }
					c.addTableWidget(field, row);

					/* pkt_sz */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_pktsz", scope, i),
						"options": ["2.5", "5", "5.5", "10", "11", "20", "30", "40", "50", "60"]
					};
					c.addTableWidget(field, row);
					setPtksz(scope, i);

					/* payload */
					field = {
						"type": "text",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_payload", scope, i),
						"cssClass": "voipQualityPayload payload_" + scope,
						"validator":
						{
							"required": function(x) {
								return function() {
									return $($.sprintf("#sys_voip_quality_%s_codec%s_type", scope, x)).val() != "none";
								}
							}(i),
							"voipPayload": true,
							"uniqueValue": ".payload_" + scope
						}
					};
					c.addTableWidget(field, row);

					/* bitpack */
					field = {
						"type": "select",
						"name": $.sprintf("sys_voip_quality_%s_codec%s_bitpack", scope, i),
						"options": "rtp aal2"
					};
					c.addTableWidget(field, row);
					setBitpackReadonly(scope, i);

                    if (i == (num - 1)) {
                        setUniqueType(scope, i);
                    }
				}
			};

			c.addTableHeader("Priority*|Type**|Packetization time (ms)|Payload|Bitpack");
			c.addTableTfootStr("* 0 - max priority.", 5);
            c.addTableTfootStr("** Each codec can be selected only once.", 5);

			c.addTitle("Internal", {"internal": true, "colspan": 5});
			addCodecsWidgets(codecs.length - 1, "int");

			c.addTitle("External", {"internal": true, "colspan": 5});
			addCodecsWidgets(codecs.length - 1, "ext");

			c.addSubmit({
				/* remove disabled attribute */
				"preSubmit": function() {
					$("[disabled]").removeAttr("disabled");
				},
				/* return disabled attribute back on readonly elements */
				"onSubmit": function() {
					$("[readonly]").attr("disabled", true);
				}
			});
		}
	});

	page.generateTabs();
};

/* Echo, FXO, FXS, VF */
Controllers.voipEcho = function() {
	var page = this.Page();

	page.addTab({
		"id": "wlec",
		"name": "Echo",
		"func": function() {
			var c = page.addContainer("wlec");
            var colNum = 6;
			c.setSubsystem("svd-wlec");
			c.addTitle("Window-based Line Echo Canceller", {"colspan": colNum});

			c.addTableHeader("Channel|Type|WLEC type|NLP|Near-end window|Far-end window");
            c.addTableTfootStr("WLEC type: ", colNum);
            c.addTableTfootStr("- NE: near-end only.", colNum);
            c.addTableTfootStr("- NFE: near-end and far-end.", colNum);
            c.addTableTfootStr("NLP: Non-linear processing.", colNum);
            c.addTableTfootStr("Near-end window: Near-end window (narraw band).", colNum);
            c.addTableTfootStr("Far-end window: Far-end window (narrow band).", colNum);

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

                field = {
					"type": "html",
					"name": channel[0] + "_type",
					"str": channel[1]
				};
				c.addTableWidget(field, row);

				/* Type, by default for VF is OFF, for others is NE */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_wlec_%s_type", channel[0]),
					"options": "off NE NFE",
					"defaultValue": channel[1] == "VF" ? "off" : "NE",
                    "onChange": function() {
                        var values = $("#" + this.id).val() == "NFE" ? "4 6 8" : "4 6 8 16";
                        $($.sprintf("#sys_voip_wlec_%s_new_nb", channel[0])).setOptionsForSelect({
                                "options": values
                        });
                        $($.sprintf("#sys_voip_wlec_%s_few_nb", channel[0])).setOptionsForSelect({
                                "options": values
                        });
                    }
				};
				c.addTableWidget(field, row);

				/* NLP, by default for FXO is ON, for others is OFF */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_wlec_%s_nlp", channel[0]),
					"options": "off on",
					"defaultValue": channel[1] == "FXO" ? "on" : "off"
				};
				c.addTableWidget(field, row);

				/* Near-end window NB */
                var type = $($.sprintf("#sys_voip_wlec_%s_type", channel[0])).val();
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_wlec_%s_new_nb", channel[0]),
                    "options": type == "NFE" ? "4 6 8" : "4 6 8 16",
					"defaultValue": "4"
				};
				c.addTableWidget(field, row);

				/* Far-end window NB */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_wlec_%s_few_nb", channel[0]),
                    "options": type == "NFE" ? "4 6 8" : "4 6 8 16",
					"defaultValue": "4"
				};
				c.addTableWidget(field, row);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

/* Dial mode, FXO */
Controllers.voipDialMode = function() {
	var page = this.Page();

	page.addTab({
		"id": "fxo",
		"name": "Dial mode",
		"func": function() {
			var c = page.addContainer("fxo");
			c.setSubsystem("svd-fxo");
			c.addTitle("Dial mode settings for FXO channels", {"colspan": 3});

			c.addTableHeader("Channel|Type|PSTN type*");
            c.addTableTfootStr("tone/pulse - tone or pulse.", 3);
            c.addTableTfootStr("pulse - pulse only.", 3);

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

                /* only FXO channels */
                if (channel[1] != "FXO") {
                    return true;
                }

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0]
				};
				c.addTableWidget(field, row);

                field = {
					"type": "html",
					"name": channel[0] + "_type",
					"str": channel[1]
				};
				c.addTableWidget(field, row);

				/* PSTN_type */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_fxo_%s_pstn_type", channel[0]),
					"options": {"tone": "tone/pulse", "pulse": "pulse"}
				};
				c.addTableWidget(field, row);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};

/* Jitter buffer, FXO, FXS, VF */
Controllers.voipJitterBuffer = function() {
	var page = this.Page();

	page.addTab({
		"id": "jitterBuffer",
		"name": "Jitter Buffer",
		"func": function() {
			var c = page.addContainer("jitterBuffer");
            var colNum = 9;
			c.setSubsystem("svd-jb");
			c.addTitle("Jitter Buffer settings", {"colspan": colNum});

			c.addTableHeader("Channel|JB Type|Pkt.Adpt.|LAT|nScaling|nInit|nMin|nMax");
            c.addTableTfootStr("JB Type: jitter buffer type.", colNum);
            c.addTableTfootStr("Pkt.Adpt.: packet adaptation.", colNum);
            c.addTableTfootStr("LAT: Local Adaptation Type:", colNum);
            c.addTableTfootStr(" - SI: on wtih sample interpollation.", colNum);
            c.addTableTfootStr("nScaling (16-255): scaling factor multiplied by 16. An increase of the scaling factor will eventually lead to an increased play out delay.", colNum);
            c.addTableTfootStr("nInit: initial size of the jitter buffer in timestamps of 125 us, nMin <= nInit <= nMax.", colNum);
            c.addTableTfootStr("nMin: minimum size of the jitter buffer in timestamps of 125 us.", colNum);
            c.addTableTfootStr("nMax: maximum size of the jitter buffer in timestamps of 125 us:", colNum);

            var mutableAttrs = ["n_scaling", "n_init_size", "n_min_size", "n_max_size"];

            var onTypeChange = function(channel) {
                var type = $($.sprintf("#sys_voip_jb_%s_type", channel)).val();

                /* set/unset local_at read-only */
                $($.sprintf("#sys_voip_jb_%s_local_at", channel))
                        .setSelectReadonly(type == "adaptive" ? false : true);

                if (type == "fixed") {
                    $(mutableAttrs).each(function(num, attr) {
                        $($.sprintf("#sys_voip_jb_%s_%s", channel, attr)).attr("readonly", true);
                    });
                } else {
                    $(mutableAttrs).each(function(num, attr) {
                        $($.sprintf("#sys_voip_jb_%s_%s", channel, attr)).removeAttr("readonly");
                    });
                }
            }

			var channels = config.getCachedOutput("voipChannels").split("\n");
			$.each(channels, function(num, record) {
				var field;
				if (record.length == 0) {
                    return true;
                }
				var row = c.addTableRow();

				/* channel[0] — number of channel, channel[1] — type of channel */
				var channel = record.split(":");

				field = {
					"type": "html",
					"name": channel[0],
					"str": channel[0] + " " + channel[1]
				};
				c.addTableWidget(field, row);

                /* type */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_jb_%s_type", channel[0]),
					"options": {"fixed": "Fixed", "adaptive": "Adaptive"},
					"defaultValue": "fixed",
                    "onChange": function() {
                        onTypeChange(channel[0]);
                    }
				};
				c.addTableWidget(field, row);

				/* pkt_adpt */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_jb_%s_pkt_adpt", channel[0]),
					"options": {"voice": "Voice", "data": "Data"},
					"defaultValue": "voice"
				};
				c.addTableWidget(field, row);

				/* local_at */
				field = {
					"type": "select",
					"name": $.sprintf("sys_voip_jb_%s_local_at", channel[0]),
					"options": "off on SI",
					"defaultValue": "off"
				};
				c.addTableWidget(field, row);

				/* n_scaling */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_jb_%s_n_scaling", channel[0]),
					"defaultValue": "22",
                    "validator": {"min": 16, "max": 255}
				};
				c.addTableWidget(field, row);

                /* n_init_size */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_jb_%s_n_init_size", channel[0]),
					"defaultValue": "20",
                    "validator": {"dynamicRange": [
                            $.sprintf("#sys_voip_jb_%s_n_min_size", channel[0]),
                            $.sprintf("#sys_voip_jb_%s_n_max_size", channel[0])
                        ]
                    }
				};
				c.addTableWidget(field, row);

                /* n_min_size */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_jb_%s_n_min_size", channel[0]),
					"defaultValue": "10",
                    "validator": {"min": 0}
				};
				c.addTableWidget(field, row);

                /* n_max_size */
				field = {
					"type": "text",
					"name": $.sprintf("sys_voip_jb_%s_n_max_size", channel[0]),
					"defaultValue": "100",
                    "validator": {"min": 0}
				};
				c.addTableWidget(field, row);

                onTypeChange(channel[0]);
			});

			c.addSubmit();
		}
	});

	page.generateTabs();
};