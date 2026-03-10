L.ui.view.extend({
	board: L.rpc.declare({
		object: 'system',
		method: 'info',
		expect: {
			memory: {},
		},
	}),

	get_ipaddr: L.rpc.declare({
		object: 'network.interface.wan',
		method: 'status',
		expect: { }
	}),

	callSet: L.rpc.declare({
		object: 'uci',
		method: 'set',
		params: ['config', 'section', 'values']
	}),

	callCommit: L.rpc.declare({
		object: 'uci',
		method: 'commit',
		params: ['config']
	}),

	do_cmd: L.rpc.declare({
        object: 'web.advance',
        method: 'cmd',
        params: [ 'cmd' ]
    }),

	get_status: L.rpc.declare({
		object: 'web.network',
		method: 'getstatus',
		expect: { }
	}),

	getFactoryInfo: L.rpc.declare({
		object: 'web.system',
		method: 'get_factory_info',
		params: ['name'],
		expect: { '': { } }
	}),

	lan_ip: L.rpc.declare({
		object: 'network.interface.lan',
		method: 'status',
		expect: { 'ipv4-address': [] },
		filter: function (data) {
			var ip = '0';
			if (data[0]) {
				ip = data[0].address;
			}
			return ip;
		},
	}),

	execute: function() {
		var self = this;
		if(L.globals.vendorName && L.globals.vendor.logo){
            $(".logo img").attr("src",`../../luci2/vendor/${L.globals.vendorName}/images/${L.globals.vendor.logo}`);
        }
		L.ui.set_vendor("corporateName",".loginname");
		L.ui.set_vendor("footerInfo",".footer>div");

		// 设置时区
		var m = new L.cbi.Map('system', {
			collabsible: true
		});

		var s = m.section(L.cbi.TypedSection, 'system', {});

		var t = s.option(L.cbi.DummyValue, '__time', {
			caption:     L.tr('Local Time')
		});

		t.load = function(sid)
		{
			var id = this.id(sid);
			var uci = this.ucipath(sid);
			self.timezone = m.get(uci.config, uci.section, 'timezone');

			if (self.interval)
				return;

			L.system.getLocaltime().then(function(info) {
				var date = new Date();
				date.setFullYear(info.year);
				date.setMonth(info.mon);
				date.setDate(info.day);
				date.setHours(info.hour);
				date.setMinutes(info.min);
				date.setSeconds(info.sec);

				self.time = Math.floor(date.getTime() / 1000);

				self.interval = window.setInterval(function() {
					date.setTime(++(self.time) * 1000);
					$('#' + id).text('%04d/%02d/%02d %02d:%02d:%02d %s'.format(
						date.getFullYear(),
						date.getMonth() + 1,
						date.getDate(),
						date.getHours(),
						date.getMinutes(),
						date.getSeconds(),
						self.timezone
					));
				}, 1000);
			});
		};

		var z_time = s.option(L.cbi.ListValue, 'zonename', {
			caption:	L.tr('Timezone'),
			initial:	'UTC'
		});

		z_time.load = function(sid) {
			z_time.value('UTC');
			return $.getJSON(L.globals.resource + '/zoneinfo.json').then(function(zones) {
				var znames = [ ];

				for (var i = 0; i < zones.length; i++)
					for (var j = 5; j < zones[i].length; j++)
						znames.push('(' + zones[i][1] + ') ' + zones[i][j]);

				znames.sort();

				for (var i = 0; i < znames.length; i++)
					z_time.value(znames[i]);

				z_time.zones = zones;
			});
		};

		z_time.save = function(sid){
			var uci = this.ucipath(sid);
			var val = this.formvalue(sid).split(') ')[1];

			if (!this.callSuper('save', sid)){
				showPage("page_wan");
				return false;
			}

			for (var i = 0; i < z_time.zones.length; i++)
				for (var j = 5; j < z_time.zones[i].length; j++)
					if (z_time.zones[i][j] == val)
					{
						m.set(uci.config, uci.section, 'timezone', z_time.zones[i][0]);
						showPage("page_wan");
						return true;
					}

			m.set(uci.config, uci.section, 'timezone', 'UTC');
			showPage("page_wan");
			return true;
		};

		m.insertInto('#time_set').then(function(){
			L.uci.callLoad('luci2').then(function(data) {
				var lang = data["main"].lang;
				$("#page_time").find(".common_button_long").text(L.tr("Next"));
				if(lang == "de") {
					$("#page_time").find(".common_button_long").css("right","140px");
				}
			})
		})

		// 模式选择
		var m_wan = new L.cbi.Map('network', {
			emptytop:    '0px'
		});

		var s_wan = m_wan.section(L.cbi.NamedSection, 'wan', {});

		var wanstatus = s_wan.option(L.cbi.DummyValue, '__wanstatus', {
			caption:        L.tr("WAN status"),
			initial:        L.tr("Testing")
		});

		function start_wanstatus_check(id)
		{
			clearInterval(L.wanstatus_interval);
			L.wanstatus_interval = setInterval(function() {
				if(L.getHash("view") != "guide"){
					clearInterval(L.wanstatus_interval);
					return;
				}
				var protonow = L.uci.get('network', 'wan', 'proto');
				if (protonow != "bridge") {
					self.get_status().then(function(info) {
						var stat = info.status;
						var ipaddrnow = L.uci.get('network', 'wan', 'ipaddr');
						if (stat != undefined) {
							if (stat == 0 ) {
								$('#' + id).text(L.tr("The network cable is not connected"));
							} else if(protonow == 'static') {
								$('#' + id).text(L.tr("The network cable is connected"));
							} else {
								self.get_ipaddr().then(function(data) {
									if(data){
										var ipexist = data["ipv4-address"];
										if (protonow == 'pppoe'){
											if(ipexist){
												$('#' + id).text(L.tr("pppoe connect success"));
											} else {
												$('#' + id).text(L.tr("pppoe connect failed"));
											}
										} else {
											if(ipexist){
												$('#' + id).text(L.tr("Connected and can go online normally"));
											} else {
												$('#' + id).text(L.tr("Can't connect to the internet"));
											}
										}
									} else {
										//if not get result
										$('#' + id).text(L.tr("Testing"));
									}
								});
							}
						} else {
							//if not get result
							$('#' + id).text(L.tr("Testing"));
						}
					});
				} else {
					$('#' + id).text(L.tr("Currently in bridge mode"));
				}
			}, 2000);
		};

		wanstatus.load = function(sid)
		{
			var id = this.id(sid);
			start_wanstatus_check(id);
		}

		p = s_wan.option(L.cbi.ListValue, 'proto', {
			caption:	L.tr("Mode"),
			initial:	'dhcp'
		})
			.value('dhcp', L.tr("DHCP"))
			.value('static', L.tr("Static"))
			.value('pppoe', L.tr("PPPoE"))
			.value('bridge', L.tr("Bridge"))

		p.save = function(sid){
			var value = this.formvalue(sid);
			var ori_value = L.uci.get('network', 'wan', 'proto');
			L.uci.set('network', 'wan', 'origin_proto',ori_value);
			if(ori_value == 'bridge' && value != 'bridge'){
				L.uci.unset('network', 'wan', 'disabled');
				L.uci.set('network', 'lan', 'ifname', 'eth0.1');
				L.uci.set('network', 'lan', 'proto', 'static');
				L.uci.load('dhcp').then(function(data) {
					L.uci.unset('dhcp', 'lan', 'ignore');
					L.uci.save();
				});
			}
			if(ori_value == 'pppoe' && value != 'pppoe'){
				L.uci.set('network', 'wan6', 'reqprefix', 'auto');
				L.uci.set('network', 'wan6', 'reqaddress', 'try');
				L.uci.load('dhcp').then(function(data) {
					L.uci.set('dhcp', 'wan', 'dhcpv6', 'relay');
					L.uci.set('dhcp', 'wan', 'ra', 'relay');
					L.uci.set('dhcp', 'wan', 'ndp', 'relay');
					L.uci.set('dhcp', 'wan', 'master', '1');
					L.uci.set('dhcp', 'lan', 'ndp', 'relay');
					L.uci.set('dhcp', 'lan', 'ra_management', '1');
					L.uci.set('dhcp', 'lan', 'dhcpv6', 'relay');
					L.uci.set('dhcp', 'lan', 'ra', 'relay');
					L.uci.unset('dhcp', 'lan', 'ra_default');
					L.uci.save();
				});
			}
			if(ori_value != 'pppoe' && value == 'pppoe'){
				L.uci.unset('network', 'wan6', 'reqprefix');
				L.uci.unset('network', 'wan6', 'reqaddress');
				L.uci.load('dhcp').then(function(data) {
					L.uci.unset('dhcp', 'wan', 'dhcpv6');
					L.uci.unset('dhcp', 'wan', 'ra');
					L.uci.unset('dhcp', 'wan', 'ndp');
					L.uci.unset('dhcp', 'wan', 'master');
					L.uci.unset('dhcp', 'lan', 'ndp');
					L.uci.unset('dhcp', 'lan', 'ra_management');
					L.uci.set('dhcp', 'lan', 'dhcpv6', 'server');
					L.uci.set('dhcp', 'lan', 'ra', 'server');
					L.uci.set('dhcp', 'lan', 'ra_default', '1');
					L.uci.save();
				});
			}
			if(value == 'pppoe'){
				var id = wanstatus.id(sid);
				L.uci.unset('network', 'wan', 'ipaddr');
				L.uci.unset('network', 'wan', 'gateway');
				L.uci.unset('network', 'wan', 'netmask');
				L.uci.set('network', 'wan', 'proto', 'pppoe');
				var ppp_times = 0;
				setTimeout(function(){
					$('#' + id).text(L.tr("Testing"));
					clearInterval(L.wanstatus_interval);
					L.ui.setting(true, L.tr("pppoe connecting..."));
					ppp_interval = setInterval(function() {
						ppp_times = ppp_times + 1;
						if(ppp_times > 10){
							clearInterval(ppp_interval);
							L.ui.setting(false);
							L.ui.setting(true, L.tr("pppoe connect failed"));
							$('body').removeClass("loading_gif");
							setTimeout(function(){
								L.ui.setting(false);
								start_wanstatus_check(id);
								$('#' + id).text(L.tr("pppoe connect failed"));
							}, 3000);
						}
						self.get_ipaddr().then(function(data){
							var ppp_status = data["ipv4-address"];
							if(ppp_status)
								ppp_status = data["ipv4-address"][0].address;
							if (ppp_status){
								clearInterval(ppp_interval);
								L.ui.setting(false);
								L.ui.setting(true, L.tr("pppoe connect success"));
								$('body').removeClass("loading_gif");
								setTimeout(function(){
									$('#' + id).text(L.tr("pppoe connect success"));
									start_wanstatus_check(id);
									L.ui.setting(false);
								}, 2000);
							}
						});
					}, 1000);
				}, 2000);
			} else if(value == 'static') {
				L.uci.unset('network', 'wan', 'auto');
				L.uci.set('network', 'wan', 'proto', 'static');
			} else if(value == 'dhcp') {
				L.uci.unset('network', 'wan', 'ipaddr');
				L.uci.unset('network', 'wan', 'gateway');
				L.uci.unset('network', 'wan', 'netmask');
				L.uci.unset('network', 'wan', 'auto');
				L.uci.set('network', 'wan', 'proto', 'dhcp');
			} else if(value == 'bridge') {
				L.uci.unset('network', 'wan', 'ipaddr');
				L.uci.unset('network', 'wan', 'gateway');
				L.uci.unset('network', 'wan', 'netmask');
				L.uci.unset('network', 'wan', 'auto');
				L.uci.set('network', 'wan', 'proto', 'bridge');
				L.uci.set('network', 'wan', 'disabled', '1');
				L.uci.set('network', 'lan', 'ifname', 'eth0.1 eth0.2');
				L.uci.set('network', 'lan', 'proto', 'dhcp');
				L.uci.load('dhcp').then(function() {
					L.uci.set('dhcp', 'lan', 'ignore', '1');
					L.uci.save();
				});
			}
			L.uci.save();
		}

		var lanip='';
		self.lan_ip().then(function (ip) {
			lanip = ip;
		});

		var ipdata = s_wan.option(L.cbi.InputValue, 'ipaddr', {
			caption:      L.tr("IP address"),
			datatype:     function(str) {

			if (!L.parseIPv4(str)){
				return L.tr('Must be a valid IPv4 address');
			}
			var ip = lanip.split('.');
			var parts = str.split('.');
			var netmask = '255.255.255.0'.split('.');
			var res0 = parseInt(ip[0]) & parseInt(netmask[0]);
			var res1 = parseInt(ip[1]) & parseInt(netmask[1]);
			var res2 = parseInt(ip[2]) & parseInt(netmask[2]);
			var res3 = parseInt(ip[3]) & parseInt(netmask[3]);
			var res_gw0 = parseInt(parts[0]) & parseInt(netmask[0]);
			var res_gw1 = parseInt(parts[1]) & parseInt(netmask[1]);
			var res_gw2 = parseInt(parts[2]) & parseInt(netmask[2]);
			var res_gw3 = parseInt(parts[3]) & parseInt(netmask[3]);

			if (res0 === res_gw0 && res1 === res_gw1 && res2 === res_gw2 && res3 === res_gw3) {
				return L.tr('Must not be in the same segment as the IP address of Lan.');
			}
				return true;
			}
		}).depends('proto', function(v) { return (v == 'static');});

		e_wan = s_wan.option(L.cbi.InputValue, 'netmask', {
			caption:      L.tr("Subnet mask"),
			datatype:    'netmask4'
		}).depends('proto', function(v) { return (v == 'static');});

		function check(v) {
			if (v.length == 0) return L.tr('Field must not be empty');
			if (L.parseIPv4(v)) {
				var ip = $('#field_network_wan_wan_ipaddr').val().split('.');
				var gateway = v.split('.');
				var netmask = $('#field_network_wan_wan_netmask').val().split('.');

				var res0 = parseInt(ip[0]) & parseInt(netmask[0]);
				var res1 = parseInt(ip[1]) & parseInt(netmask[1]);
				var res2 = parseInt(ip[2]) & parseInt(netmask[2]);
				var res3 = parseInt(ip[3]) & parseInt(netmask[3]);
				var res_gw0 = parseInt(gateway[0]) & parseInt(netmask[0]);
				var res_gw1 = parseInt(gateway[1]) & parseInt(netmask[1]);
				var res_gw2 = parseInt(gateway[2]) & parseInt(netmask[2]);
				var res_gw3 = parseInt(gateway[3]) & parseInt(netmask[3]);

				if (res0 == res_gw0 && res1 == res_gw1 && res2 == res_gw2 && res3 == res_gw3)
					return true;

				else return L.tr('Must be in the same segment as the IP address.');
			} else return L.tr('Must be a valid IPv4 address');
		}

		var gateway = s_wan.option(L.cbi.InputValue, 'gateway', {
			caption:      L.tr("Default gateway"),
			datatype:    function(v) {
				return check(v);
			}
		}).depends('proto', function(v) { return (v == 'static');});

		e_wan = s_wan.option(L.cbi.InputValue, 'username', {
			caption:      L.tr("Broadband account")
		}).depends('proto', function(v) { return (v == 'pppoe');});

		e_wan = s_wan.option(L.cbi.PasswordValue, 'password', {
			caption:      L.tr("Broadband password")
		}).depends('proto', function(v) { return (v == 'pppoe');});

		e_wan = s_wan.option(L.cbi.CheckboxValue, 'auto', {
			caption:      L.tr("Auto connect")
		}).depends('proto', function(v) { return (v == 'pppoe');});

		var manualdns = s_wan.option(L.cbi.CheckboxValue, '__manualdns', {
			caption:      L.tr("Manually set up DNS server"),
			optional:     true
		}).depends('proto', function(v) { return (v == 'dhcp' || v == 'pppoe');});

		manualdns.ucivalue = function(sid) {
			if (L.uci.get('network', 'wan', 'peerdns') == 0) return true;
			return false;
		};

		manualdns.save = function(sid) {
			var mdns = manualdns.formvalue(sid);
			if (p.formvalue(sid) != 'static') {
				L.uci.set('network', 'wan', 'peerdns', 1 ^ mdns);
				if (!mdns) L.uci.unset('network', 'wan', 'dns');
			}
			L.uci.save();
		};

		var dns1 = s_wan.option(L.cbi.InputValue, '__dns1', {
			caption:      L.tr("Preferred DNS server"),
			datatype:     'ip4addr'
		}).depends('__manualdns', function(v) {
			var proto = p.formvalue('wan');
			if(proto == "bridge"){
				return false;
			}
			return v || proto == "static";
		});

		var dns2 = s_wan.option(L.cbi.InputValue, '__dns2', {
			caption:      L.tr("Standby DNS server(Optional)"),
			optional:     true,
			datatype:     'ip4addr',
		}).depends('__manualdns', function(v) {
			var proto = p.formvalue('wan');
			if(proto == "bridge"){
				return false;
			}
			return v || proto == "static";
		});

		dns1.ucivalue = function(sid) {
			var dns_value = L.uci.get("network", "wan", "dns");
			var split = 0;
			if (dns_value) {
				split = dns_value.indexOf(" ");
				if (split == -1) return dns_value;
				else return dns_value.substring(0, split);
			}
		};

		dns2.ucivalue = function(sid) {
			var dns_value = L.uci.get("network", "wan", "dns");
			var split = 0;
			if (dns_value) {
				split = dns_value.indexOf(" ");
				if (split != -1) return dns_value.substring(split + 1);
			}
		};

		dns1.save = dns2.save =  function(sid) {
			var dnsv1 = dns1.formvalue(sid);
			var dnsv2 = dns2.formvalue(sid);
			if (dnsv2) dnsv1 += ' ' + dnsv2;
			if (p.formvalue(sid) != 'static') {
				if (!manualdns.formvalue(sid)) L.uci.unset('network', 'wan', 'dns');
				else L.uci.set('network', 'wan', 'dns', dnsv1);
			} else {
				L.uci.unset('network', 'wan', 'peerdns');
				L.uci.set('network', 'wan', 'dns', dnsv1);
			}
			L.uci.save();
		};

		servicename = s_wan.option(L.cbi.InputValue, 'service', {
			caption:      L.tr("Service Name"),
			optional:     true
		}).depends('proto', function(v) { return (v == 'pppoe');});

		servicename.save = function(sid) {
			var sname = servicename.formvalue(sid);
			if (p.formvalue('wan') == 'pppoe' && sname) L.uci.set('network', 'wan', 'service', sname);
			else L.uci.unset('network', 'wan', 'service');
			L.uci.save();
		};

		servername = s_wan.option(L.cbi.InputValue, 'ac', {
			caption:      L.tr("Server name"),
			optional:     true
		}).depends('proto', function(v) { return (v == 'pppoe');});

		servername.save = function(sid) {
			var svname = servername.formvalue(sid);
			if (p.formvalue('wan') == "pppoe" && svname) L.uci.set('network', 'wan', 'ac', svname);
			else L.uci.unset('network', 'wan', 'ac');
			L.uci.save();
		};

		uad = s_wan.option(L.cbi.CheckboxValue, '__useoperatoraddress', {
			caption:      L.tr("Use the ip address specified by the operator"),
			optional:     true
		}).depends('proto', function(v) { return (v == 'pppoe');});

		uad.ucivalue  = function(sid) {
			return !!L.uci.get("network", "wan", "pppd_options");
		};

		uad.save = function(sid) {
			var ua = uad.formvalue(sid);
			if (p.formvalue('wan') == 'pppoe') L.uci.set('network', 'wan', 'fixipEnb', 0 ^ ua);
			else L.uci.unset('network', 'wan', 'fixipEnb');
			L.uci.save();
		};

		operatoraddress = s_wan.option(L.cbi.InputValue, 'pppd_options', {
			caption:      L.tr("The ip address specified by the operator"),
			datatype:	  'ipaddr'
		}).depends('__useoperatoraddress', function(v) { return v && p.formvalue('wan') == 'pppoe';});

		operatoraddress.save = function(sid) {
			var oad = operatoraddress.formvalue(sid);
			if (uad.formvalue(sid) && p.formvalue('wan') == 'pppoe') L.uci.set('network', 'wan', 'pppd_options', oad);
			else L.uci.unset('network', 'wan', 'pppd_options');
			L.uci.save();
		};

		m_wan.insertInto('#wan_set').then(function () {
			L.uci.callLoad('luci2').then(function(data) {
				var lang = data["main"].lang;
				$("#page_wan").find(".common_button_long").text(L.tr("Next"));
				if(lang == "de") {
					$("#page_wan").find(".common_button_long").css("right","140px");
					$("#page_wan").find(".backclick").css("right","218px");
				} else if(lang == "it") {
					$("#page_wan").find(".backclick").css("right","185px");
				}
			})
			$('#wan_set').css('width', '800px');
			$('#node_network_wan_wan___default__').css({
				width: '700px',
				'margin-left': '30px',
			});
			$('.form-group').children('div:last-child').css({
				width: 'auto',
				'margin-top': '7px',
				'text-align': 'left',
				color: 'gray',
				'font-size': '12px',
				'margin-left': '210px',
			});
			$('#field_network_wan_wan_ipaddr').keyup(function () {
				gateway.setError($('#field_network_wan_wan_gateway'), check($('#field_network_wan_wan_gateway').val()));
			});
			$('#field_network_wan_wan_netmask').keyup(function () {
				gateway.setError($('#field_network_wan_wan_gateway'), check($('#field_network_wan_wan_gateway').val()));
			});
		});

		m_wan.on('apply',function(){
			L.ui.setting(true);
			let get_val = $("#field_network_wan_wan_proto").val();
			let origin_proto = L.uci.get('network', 'wan', 'origin_proto');
			let get_http = "";
			L.ui.callOption('basic_setting', 'vendor', 'hosts').then(function (value) {
				if (value.length > 1){
					get_http = 'http://'+value;
				}

				if(get_val == "bridge" && origin_proto !="bridge"){
					L.ui.setting(false);
					var form = $('<p />').text(L.tr(`The LAN IP will change when switching from other modes to bridge mode.Guest WiFi will be turned off.Please log in using the domain name "%s" in two minutes`).format(get_http));
					L.ui.dialog(L.tr('Tips'), form, {
							style: 'onlyConfirm',
							confirm: function() {
									L.ui.dialog(false);
									L.ui.setting(true);
									showPage("page_wireless");
							},
					});
					self.do_cmd("/usr/sbin/ip_monitor.sh &");
				}else if (get_val != "bridge" && origin_proto =="bridge"){
					L.ui.setting(false);
					var form = $('<p />').text(L.tr(`The LAN IP will change when switching from bridge mode to other modes.Please log in using the domain name "%s" in two minutes`).format(get_http));
					L.ui.dialog(L.tr('Tips'), form, {
							style: 'onlyConfirm',
							confirm: function() {
								L.ui.dialog(false);
								L.ui.setting(true);
								showPage("page_wireless");
							},
					});
					self.do_cmd("/usr/sbin/ip_monitor.sh &");
				}
			})

			self.do_cmd("/etc/init.d/dnsmasq restart; /etc/init.d/network restart").then(function(v){
				setTimeout(() => {
					L.ui.setting(false);
				}, 1000*40);
			});
		});

		// 无线设置
		var m_wireless = new L.cbi.Map('wireless', {
			emptytop:    '0px'
		});

		var s_wireless = m_wireless.section(L.cbi.NamedSection, 'default_radio0', {
			caption:      L.tr('2.4G')
		});

		var e_wireless = s_wireless.option(L.cbi.CheckboxValue, 'disabled_hostapd', {
			caption:      L.tr('Disabled Wifi'),
			optional:     true
		});

		e_wireless = s_wireless.option(L.cbi.InputValue, 'ssid', {
			caption:      L.tr('Wireless name'),
			datatype:     'ssid'
		});

		e_wireless = s_wireless.option(L.cbi.ListValue, 'encryption', {
			caption:	L.tr('Encryption method'),
			initial:	'none',
		})
		.value('none', L.tr('No encryption'))
		.value('psk+ccmp', L.tr('WPA Personal (PSK)'))
		.value('psk2+ccmp', L.tr('WPA2 Personal (PSK)'))
		.value('psk-mixed', L.tr('WPA/WPA2 Personal (PSK) mixed'))
		.value('sae', L.tr('WPA3 Personal (SAE)'))
		.value('sae-mixed', L.tr('WPA2/WPA3 mixed'));

		e_wireless.save = function (sid) {
			let radio_2_4 = $('#field_wireless_default_radio0_default_radio0_disabled_hostapd').is(':checked');
				if(radio_2_4){
					L.ui.do_cmd(`hostapd_cli -i wlan0 disable`);
				}else{
					L.ui.do_cmd(`hostapd_cli -i wlan0 enable`);
				}
			var encrypt = this.formvalue(sid);
			L.uci.set('wireless', 'default_radio0', 'encryption', encrypt);
			if (encrypt == 'sae')
				L.uci.set('wireless', 'default_radio0', 'ieee80211w', 2);
			else if (encrypt == 'sae-mixed')
				L.uci.set('wireless', 'default_radio0', 'ieee80211w', 1);
			else L.uci.unset('wireless', 'default_radio0', 'ieee80211w');
			L.uci.save();
		}

		e_wireless = s_wireless.option(L.cbi.PasswordValue, 'key', {
			caption:    L.tr('Password'),
			datatype:   'wpakey'
		}).depends('encryption', function(v) {
			return (v != 'none');
		});

		var e_wireless = s_wireless.option(L.cbi.DummyValue, '__encryption_warning', {
			caption:        L.tr(""),
			initial:        L.tr("Wi-Fi is not encrypted, and there is a risk of theft by others, so please set a Wi-Fi password in time.")
		}).depends('encryption', function(v) {
			return (v == 'none');
		});

		var s2_wireless = m_wireless.section(L.cbi.NamedSection, 'default_radio1', {
			caption:      L.tr('5G')
		});

		var e2_wireless = s2_wireless.option(L.cbi.CheckboxValue, 'disabled_hostapd', {
			caption:      L.tr('Disabled Wifi'),
			optional:     true
		});

		e2_wireless = s2_wireless.option(L.cbi.InputValue, 'ssid', {
			caption:      L.tr('Wireless name'),
			datatype:     'ssid'
		});

		e2_wireless = s2_wireless.option(L.cbi.ListValue, 'encryption', {
			caption:	L.tr('Encryption method'),
			initial:	'none'
		})
		.value('none', L.tr('No encryption'))
		.value('psk+ccmp', L.tr('WPA Personal (PSK)'))
		.value('psk2+ccmp', L.tr('WPA2 Personal (PSK)'))
		.value('psk-mixed', L.tr('WPA/WPA2 Personal (PSK) mixed'))
		.value('sae', L.tr('WPA3 Personal (SAE)'))
		.value('sae-mixed', L.tr('WPA2/WPA3 mixed'));

		e2_wireless.save = function (sid) {
			let radio_5 = $('#field_wireless_default_radio1_default_radio1_disabled_hostapd').is(':checked');
			if(radio_5){
				L.ui.do_cmd(`hostapd_cli -i wlan1 disable`);
			}else{
				L.ui.do_cmd(`hostapd_cli -i wlan1 enable`);
			}
			var encrypt = this.formvalue(sid);
			L.uci.set('wireless', 'default_radio1', 'encryption', encrypt);
			if (encrypt == 'sae')
				L.uci.set('wireless', 'default_radio1', 'ieee80211w', 2);
			else if (encrypt == 'sae-mixed')
				L.uci.set('wireless', 'default_radio1', 'ieee80211w', 1);
			else L.uci.unset('wireless', 'default_radio1', 'ieee80211w');
			L.uci.save();
			showPage("page_password");
		}

		e2_wireless = s2_wireless.option(L.cbi.PasswordValue, 'key', {
			caption:    L.tr('Password'),
			datatype:   'wpakey'
		}).depends('encryption', function(v) {
			return (v != 'none');
		});

		var e2_wireless = s2_wireless.option(L.cbi.DummyValue, '__encryption_warning', {
			caption:        L.tr(""),
			initial:        L.tr("Wi-Fi is not encrypted, and there is a risk of theft by others, so please set a Wi-Fi password in time.")
		}).depends('encryption', function(v) {
			return (v == 'none');
		});

		self.board().then(function (data) {
			m_wireless.insertInto('#wireless_set').then(function () {
				L.uci.callLoad('luci2').then(function(data) {
					var lang = data["main"].lang;
					$("#page_wireless").find(".common_button_long").text(L.tr("Next"));
					if(lang == "de") {
						$("#page_wireless").find(".common_button_long").css("right","140px");
						$("#page_wireless").find(".backclick").css("right","218px");
					} else if(lang == "it") {
						$("#page_wireless").find(".backclick").css("right","185px");
					}
				})
				$('#wireless_set').css('width', '800px');
				$('#node_wireless_default_radio0_default_radio0___default__').css({
					width: '700px',
					'margin-left': '30px',
				});
				$('#node_wireless_default_radio1_default_radio1___default__').css({
					width: '700px',
					'margin-left': '30px',
				});
				if (Number(data.total) <= 64*1024*1024) {
					$('#field_wireless_default_radio0_default_radio0_encryption').find('[value="sae"]').css('display', 'none');
					$('#field_wireless_default_radio0_default_radio0_encryption').find('[value="sae-mixed"]').css('display', 'none');
					$('#field_wireless_default_radio1_default_radio1_encryption').find('[value="sae"]').css('display', 'none');
					$('#field_wireless_default_radio1_default_radio1_encryption').find('[value="sae-mixed"]').css('display', 'none');
				}
				else {
					$('#field_wireless_default_radio0_default_radio0_encryption').find('[value="sae"]').css('display', '');
					$('#field_wireless_default_radio0_default_radio0_encryption').find('[value="sae-mixed"]').css('display', '');
					$('#field_wireless_default_radio1_default_radio1_encryption').find('[value="sae"]').css('display', '');
					$('#field_wireless_default_radio1_default_radio1_encryption').find('[value="sae-mixed"]').css('display', '');
				}
			});
		});

		// 密码设置
		var m_password = new L.cbi.Map('rpcd', {});

		var s_password = m_password.section(L.cbi.DummySection, '__login', {

		});

		var password = s_password.option(L.cbi.PasswordValue, 'password', {
			caption:     L.tr('Password'),
			optional:    true,
			datatype:    'rangelength(5,12)'
		});

		var repassword = s_password.option(L.cbi.PasswordValue, 'repassword', {
			caption:     L.tr('Confirmation'),
			optional:    true,
			datatype:     function(str, sid) {
				var password = '' + $('#field_rpcd___login___login_password').val()
				var val = '' + str;

				for(var i = 0; i < val.length; i ++)
				{
					var c = val.substr(i, 1);
					var ts = escape(c);
					if(ts.substring(0, 2) == "%u") {
						return L.tr('Must void Chinese characters');
					}
				}

				if (val.length >= 5 && val.length <= 12) {
					if(password == val) {
						return true;
					} else {
						return L.tr("Two password inputs must be consistent")
					}
				} else {
					return L.tr('Must be between 5 and 12 characters');
				}
			}
		});

		password.ucivalue = function(sid) {
			return '';
		}
		password.toggle = function(sid) {
			var id = '#' + this.id(sid);
			this.callSuper('toggle', sid);
		};

		function handleLogin() {
			L.ui.login().then(function() {
				history.replaceState(null, '', window.location.origin + window.location.pathname + window.location.search);
				location.reload(true);
			});
		}

		password.save = function(sid) {
			L.ui.setting(false);
			var real_sid;
			var rpcd_users = L.uci.sections('rpcd');
			for(var i = 0; i < rpcd_users.length; i++) {
				if(L.uci.get('rpcd', rpcd_users[i]['.name'], 'username') == 'root') {
					real_sid = rpcd_users[i]['.name'];
					break;
				}
			}

			var pw = password.formvalue(sid);
			var repw = repassword.formvalue(sid);

			if(pw == undefined){
				L.ui.dialog(L.tr('Tips'), L.tr('Please input a password'), {
					style: 'close'
				});
				return
			}else if(repw == undefined){
				L.ui.dialog(L.tr('Tips'), L.tr('Please enter the confirmation password'), {
					style: 'close'
				});
				return
			}else if(repw != pw){
				L.ui.dialog(L.tr('Tips'), [
					$('<p />').text(L.tr('Two password inputs must be consistent'))
				], { style: 'close' });
				return
			}

			if (!pw || !real_sid)
				return;
			return L.ui.cryptPassword(pw).then(function(crypt) {
				self.callSet('rpcd', real_sid, { 'password': crypt });
				self.callSet('basic_setting', 'web_guide', { 'inited': '1' });
				self.callCommit('rpcd');
				self.callCommit('basic_setting');
				L.ui.callOption('basic_setting', 'web_guide', 'hascountryid').then(function (value) {
					if (value == '0'){
						L.uci.callLoad('luci2').then(function(data) {
							var lang = data["main"].lang;
							self.getFactoryInfo('countryid').then(function(info){
								var countryid = info.result;
								if(countryid == "UK" && lang != "en") {
									switch (lang)
									{
									case 'de':
										var new_countryid="DE";
										break;

									case 'es':
										var new_countryid="ES";
										break;

									case 'fr':
										var new_countryid="FR";
										break;

									case 'it':
										var new_countryid="IT";
										break;
									}
									var countryPromises = [
										self.callSet('wireless', 'radio0', { 'country': new_countryid }),
										self.callSet('wireless', 'radio1', { 'country': new_countryid })
									];
									Promise.all(countryPromises).then(function() {
										self.callCommit('wireless');
									})
								}

								handleLogin();
							})
						});
					} else{
						handleLogin();
					}
				})
			});
		};

		m_password.on('save', function() {
			L.uci.changes().then(function(changes) {
				self.relogin = true;
			});
		});

		m_password.insertInto('#password_set').then(function(){
			L.uci.callLoad('luci2').then(function(data) {
				var lang = data["main"].lang;
				$("#page_password").find(".common_button_long").text(L.tr("Finish"));
				$("#page_password").find(".common_button_long").css("right","46px");
				if(lang == "es") {
					$("#page_password").find(".backclick").css("right","160px");
				} else if(lang == "it") {
					$("#page_password").find(".backclick").css("right","155px");
				}
			})
		});

		function showPage(pageId) {
			const pages = document.querySelectorAll('.guidepage');
			pages.forEach(page => {
				page.classList.remove('page_active');
			});

			const activePage = document.getElementById(pageId);
			if (activePage) {
				activePage.classList.add('page_active');
			}

			localStorage.setItem('currentStep', pageId);
		}

		const currentStep = localStorage.getItem('currentStep') || 'page_time';
		showPage(currentStep);

		$(".backclick").off('click').on("click",function(){
			let elemb = $(this).attr("data-id");
			showPage(elemb);
		});

		$(".nextclick").off('click').on("click",function(){
			let elemn = $(this).attr("data-id");
			showPage(elemn);
		});
	}
});