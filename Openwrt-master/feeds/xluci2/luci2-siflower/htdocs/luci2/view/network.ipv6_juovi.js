L.ui.view.extend({
	getWan6status: L.rpc.declare({
		object: 'network.interface.wan_6',
		method: 'status',
		expect: { '': {} }
	}),

	getWanstatus: L.rpc.declare({
		object: 'network.interface.wan6',
		method: 'status',
		expect: { '': {} }
	}),

	getLanstatus: L.rpc.declare({
		object: 'network.interface.lan',
		method: 'status',
		expect: { '': {} }
	}),

	execute: function() {
		var self = this;
		L.rpc.batch();
		self.getWan6status();
		self.getWanstatus();
		self.getLanstatus()
		L.rpc.flush().then(function(status) {
			var wan6_status = status[0];
			var wan_status = status[1];
			var lan_status = status[2];
			var m = new L.cbi.Map('network', {
				caption:        L.tr('IPV6'),
				description: L.tr("You can view IPV6 related configurations and information."),
				collabsible: true,
				pageaction: false,
			});
			var s = m.section(L.cbi.NamedSection, 'wan6', {
				caption:      L.tr('WAN')
			});

			var p = s.option(L.cbi.DummyValue, 'proto', {
				caption:        L.tr('Proto')
			})

			p.ucivalue = function(sid){
				var proto = L.uci.get("network","wan6","proto");
				if(proto == "static"){
					return L.tr('Manual');
				}else{
					return L.tr('Auto');
				}
			};

			var manualdns = s.option(L.cbi.DummyValue, '__dns', {
				caption:        L.tr('Type Of DNS'),
			})

			manualdns.ucivalue = function(sid){
				var ifdns = L.uci.get("network","wan6","dns");
				if(ifdns){
					return L.tr('Manual');
				}else{
					return L.tr('Auto');
				}
			};

			var wan_ipaddr = s.option(L.cbi.DummyValue, '_wan_ipaddr', {
				caption:        L.tr('IPV6 Address')
			})

			wan_ipaddr.ucivalue = function(sid){
				if(wan6_status.up == true) {
					return wan6_status["ipv6-address"][0].address + '/' + wan6_status["ipv6-address"][0].mask;
				} else if(wan_status.up == true) {
					return wan_status["ipv6-address"][0].address + '/' + wan_status["ipv6-address"][0].mask;
				} else {
					return null;
				}
			};

			var wan_dns = s.option(L.cbi.DummyValue, '_wan_dns', {
				caption:        L.tr('IPV6 DNS Server')
			})

			wan_dns.ucivalue = function(sid) {
				if(wan6_status.up == true) {
					return wan6_status["dns-server"].join(", ");
				} else if(wan_status.up == true) {
					return wan_status["dns-server"].join(", ");
				} else {
					return null;
				}
			};

			var d = new L.cbi.Map("dhcp", {
				emptytop: '0px',
				pageaction: false,
			});

			var l = d.section(L.cbi.NamedSection, "lan", {
				caption: L.tr("LAN"),
			});
			var rd = l
				.option(L.cbi.DummyValue, "__ra", {
					caption: L.tr("Router Broadcast"),
					datatype: "string",
				})

			rd.ucivalue = function (sid) {
				var ra = L.uci.get("dhcp", "lan", "ra_default");
				if (ra == undefined || ra == "0") return L.tr("Disable");
				else return L.tr("Auto");
			};

			var dm = l.option(L.cbi.DummyValue, "__dhcpmode", {
				caption: L.tr("DHCP Mode"),
			})

			dm.ucivalue = function (sid) {
				var lan_dhcpv6 = L.uci.get("dhcp", "lan", "dhcpv6");
				var wan_ip6prefix = L.uci.get("network", "wan6", "ip6prefix");
				if (lan_dhcpv6 == "server" || lan_dhcpv6 == "relay") {
					if (wan_ip6prefix == undefined || wan_ip6prefix == "auto") {
						return L.tr("Auto");
					} else {
						return L.tr("Manual");
					}
				} else if (lan_dhcpv6 == "disabled") {
					return L.tr("Disable");
				};
			}

			var lan_ipaddr = l.option(L.cbi.DummyValue, '_lan_ipaddr', {
				caption:        L.tr('IPV6 Address')
			})

			lan_ipaddr.ucivalue = function(sid){
				if (lan_status.up === true) {
					if (lan_status["ipv6-prefix-assignment"] && lan_status["ipv6-prefix-assignment"][0] && lan_status["ipv6-prefix-assignment"][0]["local-address"]) {
						return lan_status["ipv6-prefix-assignment"][0]["local-address"].address + '/' + lan_status["ipv6-prefix-assignment"][0]["local-address"].mask;
					} else {
						return null;
					}
				} else {
					return null;
				}
			};

			var lan_prefix = l.option(L.cbi.DummyValue, '_lan_prefix', {
				caption:        L.tr('IPV6 Prefix')
			})

			lan_prefix.ucivalue = function(sid){
				if (lan_status.up == true) {
					if (lan_status["ipv6-prefix-assignment"] && lan_status["ipv6-prefix-assignment"][0]) {
						return lan_status["ipv6-prefix-assignment"][0].address;
					} else {
						return null;
					}
				} else {
					return null;
				}
			};

			m.insertInto("#map");
			d.insertInto("#map2");
		})
	},
});