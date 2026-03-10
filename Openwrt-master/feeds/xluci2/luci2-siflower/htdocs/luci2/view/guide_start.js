L.ui.view.extend({
    execute: function() {
        L.ui.showLangSelect();

        if(L.globals.vendorName === "juovi"){
            $(".clickSkip").hide();
            $(".agreement").show();
            $(".skipInfo").hide();
        }

        if(L.globals.vendorName && L.globals.vendor.logo){
            $(".logo img").attr("src",`../../luci2/vendor/${L.globals.vendorName}/images/${L.globals.vendor.logo}`);
        }

        L.ui.set_vendor("corporateName",".loginname");
        L.ui.set_vendor("maintitle",".guide_h");
        L.ui.set_vendor("footerInfo",".footer>div");

        $(".clickSkip").on('click',function(){
            L.ui.callSet('basic_setting', 'web_guide', { 'inited': '1' });
            L.ui.callCommit('basic_setting');
            history.replaceState(null, '', window.location.origin + window.location.pathname + window.location.search);
            L.ui.login();
            location.reload(true)
        })

        $(".clickOk").off('click').on('click',function(){
            if(L.globals.vendorName === "juovi"){
                let check = $('#checkout_privacy').is(':checked');
                if(check){
                    L.ui.loadHtml("guide","body");
                }else{
                    L.ui.dialog(L.tr('Tips'), L.tr('Please check and agree to the Privacy Policy and User Terms'), {
                        style: 'close'
                    });
                    return
                }
            }else{
                L.ui.loadHtml("guide","body");
            }
        })

        function showAgreement(url,tips){
            $.ajax(L.globals.resource + `/template/agreement/${url}.htm`, {
                cache: true,
                dataType: 'text',
                success: function (data) {
                    data = data.replace(/<%([#:=])?(.+?)%>/g, function(match, p1, p2) {
                        p2 = p2.replace(/^\s+/, '').replace(/\s+$/, '');
                        switch (p1)
                        {
                        case '#':
                            return '';

                        case ':':
                            return L.tr(p2);

                        case '=':
                            return L.globals[p2] || '';

                        default:
                            return '(?' + match + ')';
                        }
                    });
                    L.ui.dialog(L.tr(tips), data, {
                        style: 'close'
                    });
                },
            })
        }

        $("#privacy_info").off('click').on('click',function(){
            showAgreement(`privacy_${L.globals.currlang}`,"Privacy Policy");
        })

        $("#user_info").off('click').on('click',function(){
            showAgreement(`user_${L.globals.currlang}`,"Terms of Use");
        })
    }
})