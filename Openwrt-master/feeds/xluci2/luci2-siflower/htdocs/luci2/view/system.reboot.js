L.ui.view.extend({
	get_routert: L.rpc.declare({
		object: 'system',
		method: 'info',
		expect: {},
		filter: function (data) {
			if (data) return data.localtime;
		}
	}),
	cmd: L.rpc.declare({
		object: 'web.advance',
		method: 'cmd',
		params: ['cmd'],
	}),
	read: L.rpc.declare({
		object: 'file',
		method: 'read',
		params: ['path'],
		expect: {
			data: '',
		},
	}),
	write: L.rpc.declare({
		object: 'file',
		method: 'write',
		params: ['path', 'data', 'append'],
	}),

	handleReboot: function () {
		var form = $('<p />').text(L.tr('Really reboot the device?'));
		L.ui.dialog(L.tr('Reboot'), form, {
			style: 'confirm',
			confirm: function () {
				L.system.performReboot().then(function () {
					L.ui.reconnect(L.tr('Device rebooting...'));
				});
			},
		});
	},

	execute: function () {
		var self = this, localt, routert, fit = false;

		$('#btn_reboot').click(self.handleReboot);
		$('.tableICell').prop('sel', 0).css('background-color', 'white');

		getrules();
		timefail();

		function flag(now) {
			if (!$(now).prop('sel'))
				$(now).css('background-color', '#FB6666');
			else
				$(now).css('background-color', 'white');
			$(now).prop('sel', !$(now).prop('sel'));
		}

		function chosen(here, x, y) {
			var now;
			if (!here) {
				if (!x) {
					for (var a = 1; a < 25; a++) {
						now = $('#weekTb')
							.find('tr:nth-child(' + y + ')')
							.find('td:nth-child(' + a + ')')
							.find('.tableICell');
						flag(now);
					}
				} else if (!y) {
					for (var a = 1; a < 8; a++) {
						now = $('#weekTb')
							.find('tr:nth-child(' + a + ')')
							.find('td:nth-child(' + x + ')')
							.find('.tableICell');
						flag(now);
					}
				} else {
					now = $('#weekTb')
						.find('tr:nth-child(' + y + ')')
						.find('td:nth-child(' + x + ')')
						.find('.tableICell');
					flag(now);
				}
			} else {
				now = here;
				flag(now);
			}
		}

		function getrules() {
			self.read('/etc/crontabs/root').then(function (r) {
				var rules = r.split('\n');
				if (rules) {
					for (var i = 0; i < rules.length; i++) {
						if (rules[i].trim() === '') continue;
						var temp = rules[i].split(' ');
						if (temp.length >= 5 && temp[temp.length - 1] == '/bin/sf_reboot.sh') {
							var time = rules[i].split(' ').splice(0, 5).map(Number);
							time[4] = (time[4] + 6) % 7;
							chosen(0, time[1] + 1, time[4] + 1);
						}
					}
				}
			});
		}

		function timefail() {
			self.get_routert().then(function (time) {
				localt = Math.trunc(Date.now() / 1000);
				routert = time - 8 * 60 * 60;
				if (routert < localt + 30 && routert > localt - 30) fit = true;
				if (!fit) $('#warning').show();
				else $('#warning').hide();
			});
		}

		// 设置计划任务
		function setrules() {
			var status = false;
			let rules = new Array();
			for (var i = 1; i < 8; i++) {
				var now = $('#weekTb')
					.find('tr:nth-child(' + i + ')')
					.find('i:first');
				for (var j = 0; j < 24; j++) {
					if ($(now).prop('sel') != status) {
						var w = i % 7;
						if (!status)
							rules.push('0 ' + j + ' * * ' + w + ' /bin/sf_reboot.sh');
						status = !status;
					}
					now = $(now).parent().next().children();
				}
			}
			self.cmd('sed -i "/sf_reboot.sh/d" /etc/crontabs/root');
			rules = rules.join('\n');
			self.write('/etc/crontabs/root', rules + '\n', true);
			self.cmd('/etc/init.d/cron restart');
		}

		$('.tableICell').click(function () {
			chosen(this);
		});

		$('.tableICell').on('mousedown', function () {
			$('.tableICell').on('mouseover', function () {
				$(this).click();
			});
		});

		$('.tableICell').on('mouseup', function () {
			$('.tableICell').off('mouseover');
		});

		$('#timeWeek').find('li').click(function () {
			chosen(0, 0, $(this).index() + 1);
		});

		$('#timeHour').find('span').click(function () {
			chosen(0, $(this).parent().index() + 1);
		});

		$('#save').click(function () {
			setrules();
			L.ui.setting(L.tr('Saving...'));
			var tip = setTimeout(function () {
				clearTimeout(tip);
				L.ui.setting('', false);
			}, 1500);
		});
	}
});