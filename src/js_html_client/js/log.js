"use strict;"

var log = (function() {
    let __log = [];
    return (message) => {
        __log.unshift(message);
        if (__log.length > 10) { __log.pop(); }
        let text = "";
        if (document.getElementById("log_checkbox").checked) {
            __log.forEach(function _(this_text) { text += this_text + "<br>"; });
        }
        document.getElementById("log").innerHTML = text;
    };
}())
