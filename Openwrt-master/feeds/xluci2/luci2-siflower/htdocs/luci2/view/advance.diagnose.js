L.ui.view.extend({
	dns: L.rpc.declare({
		object: 'network.interface.wan',
		method: 'status',
		expect: {
			'dns-server': [],
		},
	}),
	cmd: L.rpc.declare({
		object: 'web.advance',
		method: 'cmd',
		params: ['cmd'],
	}),
	wifi_status: L.rpc.declare({
		object: 'network.device',
		method: 'status',
	}),
	read: L.rpc.declare({
		object: 'file',
		method: 'read',
		params: ['path'],
		expect: {
			data: '',
		},
	}),

	execute: function () {
		var self = this,
			on = false;
		let taskTimer = null;
		let taskIndex = "";
		let _vendorName = L.globals.vendorName;

		function showDataList(data){
			$("#dataList").empty();

			let typeCount = {};
            $.each(data, function(index, item) {
                typeCount[item.type] = (typeCount[item.type] || 0) + 1;
            });

			let typeRendered = {};
            $.each(data, function(index, item) {
                let html = "<tr>";

                if (!typeRendered[item.type]) {
                    html += `<td rowspan="${typeCount[item.type]}">${L.tr(item.type)}</td>`; //Determine the merging of cells of the same type
                    typeRendered[item.type] = true;
                }

				html += `
                    <td>${L.tr(item.value)}</td>
                    <td><div class="statusIcon"></div></td>
					<td><div class="statusInfo">${L.tr("To Be Diagnosed")}</div></td>
                `;
				html += "</tr>";

                $("#dataList").append(html);
            });
		}

		$.ajax(L.globals.resource + `/vendor/${L.globals.vendorName}/resources/diagnose.json`,{
			cache:    true,
			dataType: 'text',
			dataType: 'json',
			success:  function(data) {
				showDataList(data);
			}
		});

		function uplink() {
			var flag = 3, msg = 'No Connection';
			self.cmd("ping siflower.com.cn -A -w 10|grep loss|awk '{print $7}' > /tmp/ping");
			var r1 = setTimeout(function () {
				clearTimeout(r1);
				self.read('/tmp/ping').then(function (res) {
					if (res == '0%\n') {
						flag = 2;
						msg = '';
					}
					changes(flag, taskIndex, L.tr(msg));
					self.cmd('rm /tmp/ping');
				});
			});
		}

		function DNS() {
			var flag = 3, msg = 'Diagnosis Fail';
			self.dns().then(function (dns) {
				if (dns[0]) {
					flag = 2;
					msg = '';
				}
				changes(flag, taskIndex, L.tr(msg));
			});
		}

		function HTTP() {
			let flag = 3, msg = 'Diagnosis Fail';
			self.cmd('test -f /usr/bin/httping && echo "$FILE exist" > /tmp/httping');
			self.read('/tmp/httping').then(function (http) {
				if (http) {
					self.cmd("httping -g https://siflower.com.cn -c 10 | grep connects,|awk '{print $5}' > /tmp/httping");
					var r2 = setTimeout(function () {
						clearTimeout(r2);
						self.read('/tmp/httping').then(function (res) {
							if (res == '0.00%\n') {
								flag = 2;
								msg = '';
							}
							changes(flag, taskIndex, L.tr(msg));
							self.cmd('rm /tmp/httping');
						});
					}, 15000);
				}else {
					changes(flag, taskIndex, L.tr("Tools Not Installed"));
				}
				self.cmd('rm /tmp/httping');
			});
		}

		function Key() {
			var flag = 3, msg='Error', i = 5;
			L.ui.setting(1, L.tr('Please short press the button within %ds.').format(i));
			var fresh = setInterval(function () {
				i--;
				$('.loading').remove();
				if (i > 0) {
					L.ui.setting(1, L.tr('Please short press the button within %ds.').format(i));
				} else {
					clearInterval(fresh);
				}
			}, 1000);
			var delay = setTimeout(function () {
				clearTimeout(delay);
				self.read('/tmp/key').then(function (key) {
					if (key == 'pressed\n') {
						flag = 2;
						msg = '';
					}
					changes(flag, taskIndex, L.tr(msg));
					self.cmd('rm /tmp/key');
				})
			}, 5000);
		}

		function LED() {
			var flag = 3, msg = 'Error';
			L.ui.setting(false)

			L.ui.dialog(L.tr('Tips'), L.tr('Observe whether the LED blinks normally.'), {
				style: 'confirm',
				cancel: function() {
					L.ui.dialog(false)
					changes(flag, taskIndex, L.tr(msg));
				},
				confirm: function() {
					L.ui.dialog(false)
					flag = 2;
					msg = '';
					changes(flag, taskIndex, L.tr(msg));
				}
			});
		}

		function WiFi() {
			let flag = 3;

			self.wifi_status().then((wifi) => {
				L.uci.callLoad('wireless').then((data) => {
					const disabled_wlan0 = data['default_radio0']?.disabled_hostapd ?? 0;
					const disabled_wlan1 = data['default_radio1']?.disabled_hostapd ?? 0;

					const wlan0_up = wifi.wlan0?.up ?? false;
					const wlan1_up = wifi.wlan1?.up ?? false;

					const wlan0_off = !wlan0_up || disabled_wlan0 == 1;
					const wlan1_off = !wlan1_up || disabled_wlan1 == 1;

					let msg = '';

					if (!wlan0_off && !wlan1_off) {
						flag = 2;
						msg = 'All Wi-Fi On';
					} else if (wlan0_off && wlan1_off) {
						msg = 'All Wi-Fi Off';
					} else if (wlan0_off) {
						msg = 'Wi-Fi 2.4G is Off.';
					} else {
						msg = 'Wi-Fi 5G is Off.';
					}

					changes(flag, taskIndex, L.tr(msg));
				});
			});
		}

		function changes(flag, index, txt) {
			const now = $('.statusIcon').eq(index);

			if (!now.length) return;

			const styles = {
				1: { 'background-position': '49% 21.04%' }, // Diagnosing
				2: { 'background-position': '49% 43.8%' },  // Done
				3: { 'background-position': '49% 24.1%', 'background-size': '400%' } // Error
			};

			now.css(styles[flag] || {});

			if (flag === 1) {
				txt = L.tr('Diagnosing');
			} else if (flag === 2 && !txt) {
				txt = L.tr('OK');
			}

			now.closest("td").next().find("div")?.text(txt);
			if (flag !== 1) on = false;
		}

		function whendone(num, cases = null) {
			num = Number(num);

			if (cases && !cases.includes(num)) {
				whendone(num + 1, cases);
				return;
			}

			if (taskTimer) {
				clearInterval(taskTimer);
				taskTimer = null;
			}

			taskTimer = setInterval(function () {
				if (!on) {
					clearInterval(taskTimer);
					taskTimer = null;
					on = true;
					taskIndex = cases ? cases.indexOf(num) : num - 1;

					const taskMap = {
						1: uplink,
						2: DNS,
						3: HTTP,
						4: Key,
						5: LED,
						6: WiFi
					};

					if (taskMap[num]) {
						taskMap[num]();
						if (num !== 5) changes(1, taskIndex, '');
					}

					if (cases) {
						let next = cases.find(n => n > num);
						if (next) whendone(next, cases);
						else L.ui.setting(false)
					} else {
						whendone(num + 1);
					}
				}
			}, 1000);
		}

		const vendorTaskMap = {
			"default": null, //all tasks
			"hsg": null,
			"juovi": [1, 2, 6] //specify tasks
		};

		$('#diagnose').click(function () {
			L.ui.setting(1,L.tr("Diagnosing"));
			on = false;
			$(".statusIcon").css({
				"background-position": "49% 42.12%",
				"background-size": "700%"
			});
			$(".statusInfo").each(function(i,v){
				$(this).text(L.tr("To Be Diagnosed"));
			})

			whendone(1, vendorTaskMap[_vendorName] || null);
		});
	},
});