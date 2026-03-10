L.ui.view.extend({
	cmd: L.rpc.declare({
		object: 'web.advance',
		method: 'cmd',
		params: [ 'cmd' ],
	}),

    execute: function() {
        var self = this;
        var m = new L.cbi.Map('basic_setting', {
            caption: L.tr('LED setting'),
            collabsible: true
        });

        var s = m.section(L.cbi.DummySection, '__led', {
//            caption: L.tr('LEDsetting'),
        });

        var enable = s.option(L.cbi.CheckboxValue, 'enable', {
            caption: L.tr('enable LED'),
            optional: true
        });

        enable.ucivalue = function(sid) {
            var enable_value = L.uci.get("basic_setting", "led", "enable");
            if (enable_value == 1) {
                return true;
            } else {
                return false;
            }
        };

        enable.save = function(sid) {
            var enable_value = enable.formvalue(sid);
            L.uci.set('basic_setting', 'led', 'enable', enable_value);
            L.uci.save().then(function () {
                if (enable_value == 1) {
                    var cmd = "sh /sbin/internet_detect.sh 2 1";
                    self.cmd(cmd);
                } else {
                    var cmd = "echo 0 > sys/class/leds/bpi-blue/brightness";
                    self.cmd(cmd);
                    cmd = "echo 0 > sys/class/leds/bpi-green/brightness";
                    self.cmd(cmd);
                    cmd = "echo 0 > sys/class/leds/bpi-red/brightness";
                    self.cmd(cmd);
                }
            })
        };

        m.insertInto('#ledmap');
    }
});
