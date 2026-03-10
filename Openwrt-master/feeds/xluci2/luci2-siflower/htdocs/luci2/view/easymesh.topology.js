L.ui.view.extend({
    get_topo: L.rpc.declare({
        object: "file",
        method: "read",
        params: ["path"]
    }),

    file_exec: L.rpc.declare({
        object: 'file',
        method: 'exec',
        params: ['command', 'params'],
    }),

    translateLine: function(line) {
        let translatedLine = line;
        if (translatedLine.includes('bw:')) {
            let bwValue = translatedLine.split('bw:')[1].split(' ')[0];
            if (bwValue === '1') {
                translatedLine = translatedLine.replace('bw:1', 'bw:20M');
            } else if (bwValue === '2') {
                translatedLine = translatedLine.replace('bw:2', 'bw:40M');
            } else if (bwValue === '3') {
                translatedLine = translatedLine.replace('bw:3', 'bw:80M');
            }
        }
        translatedLine = translatedLine.replace('GW   name', L.tr('GW   name'));
        translatedLine = translatedLine.replace('ETH', L.tr('ETH'));
        translatedLine = translatedLine.replace('ipv4', L.tr('ip'));
        translatedLine = translatedLine.replace('prev_mac', L.tr('prev_mac'));
        translatedLine = translatedLine.replace('BACKHAUL', L.tr('BACKHAUL'));
        translatedLine = translatedLine.replace('type', L.tr('type'));
        translatedLine = translatedLine.replace('WIRED_BACKHAUL', L.tr('WIRED_BACKHAUL'));
        translatedLine = translatedLine.replace('WIRELESS_BACKHAUL', L.tr('WIRELESS_BACKHAUL'));
        translatedLine = translatedLine.replace('RADIO', L.tr('RADIO'));
        translatedLine = translatedLine.replace('bw', L.tr('bw'));
        translatedLine = translatedLine.replace('channel', L.tr('channel'));
        translatedLine = translatedLine.replace('BSS', L.tr('BSS'));
		translatedLine = translatedLine.replace('IRE', L.tr('IRE'));
        translatedLine = translatedLine.replace('NON-1905 NEIGHBOR', L.tr('NON-1905 NEIGHBOR'));
		translatedLine = translatedLine.replace('CLIENT', L.tr('CLIENT'));
        return translatedLine;
    },

    execute: function() {
        var self = this;
        self.file_exec('/bin/sf_easymesh', ["get_topo_for_web"]).then(function() {
            self.get_topo("/tmp/mesh_topo.txt").then(function(data) {
                // 分割字符串为行数组
                let lines = data.data.split('\n');
                let resultHTML = '';
                let indentLevel = 0;
                lines.forEach(line => {
                    // 处理缩进
                    let currentIndentLevel = 0;
                    while (line.startsWith('    ')) {
                        currentIndentLevel++;
                        line = line.substring(4);
                    }
                    let indent = '';
                    for (let i = 0; i < currentIndentLevel; i++) {
                        indent += '&nbsp;&nbsp;&nbsp;&nbsp;';
                    }
                    let translatedLine = self.translateLine(line);
                    resultHTML += `<p>${indent}${translatedLine}</p>`;
                });
                $("#map").append(resultHTML);
            });
        });
    }
});