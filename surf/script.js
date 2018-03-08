function initKeyBind(e){
    var t = e.target;
    if( t.nodeType == 1){
        addKeyBind( 'C-e', 'hintMode()', e );
        addKeyBind( 'C-E', 'hintMode(true)', e );
        addKeyBind( 'C-c', 'removeHints()', e );
        addKeyBind( 'C-z', 'simpleread()', e );
    }
}

// --------------------------------------------------------------------------  //

var hint_num_str = '';
var hint_elems = [];
var hint_enabled = false;

function hintMode(){
    hint_enabled = true;
    setHints();
    document.removeEventListener('keydown', initKeyBind, false);
    document.addEventListener('keydown', hintHandler, false);
    hint_num_str = '';
}

function hintHandler(e){
    e.preventDefault();  //Stop Default Event
    var pressedKey = get_key(e);
    if (pressedKey == 'Enter') {
        if (hint_num_str == '') hint_num_str = '1';
        judgeHintNum(Number(hint_num_str));
    } else if (/[0-9]/.test(pressedKey) == false) removeHints();
    else {
        hint_num_str += pressedKey;
        var hint_num = Number(hint_num_str);
        if (hint_num * 10 > hint_elems.length + 1) judgeHintNum(hint_num);
        else {
            var hint_elem = hint_elems[hint_num - 1];
            if (hint_elem != undefined && hint_elem.tagName.toLowerCase() == 'a')
                setHighlight(hint_elem, true);
        }
    }
}

function setHighlight(elem, is_active) {
    if (is_active) {
        var active_elem = document.body.querySelector('a[highlight=hint_active]');
        if (active_elem != undefined)
            active_elem.setAttribute('highlight', 'hint_elem');
        elem.setAttribute('highlight', 'hint_active');
    } else elem.setAttribute('highlight', 'hint_elem');
}

function setHintRules() {
    var ss = document.styleSheets[0];
    // THIS HIGHLIGHTS THE FULL OPTION TEXT --- ANNOYING!
    // ss.insertRule('a[highlight=hint_elem] {background-color: yellow}', 0);
    // ss.insertRule('a[highlight=hint_active] {background-color: lime}', 0);
}

function deleteHintRules() {
    var ss = document.styleSheets[0];
    ss.deleteRule(0); ss.deleteRule(0);
}

function judgeHintNum(hint_num) {
    var hint_elem = hint_elems[hint_num - 1];
    if (hint_elem != undefined) execSelect(hint_elem);
    else removeHints();
}

function execSelect(elem) {
    var tag_name = elem.tagName.toLowerCase();
    var type = elem.type ? elem.type.toLowerCase() : "";
    if (tag_name == 'a' && elem.href != '') {
        setHighlight(elem, true);
        location.href=elem.href;
    } else if (tag_name == 'input' && (type == "submit" || type == "button" || type == "reset")) {
        elem.click();
    } else if (tag_name == 'input' && (type == "radio" || type == "checkbox")) {
        elem.checked = !elem.checked;
    } else if (tag_name == 'input' || tag_name == 'textarea') {
        elem.focus();
        elem.setSelectionRange(elem.value.length, elem.value.length);
    } removeHints();
}

function setHints() {
    setHintRules();
    var win_top = window.scrollY;
    var win_bottom = win_top + window.innerHeight;
    var win_left = window.scrollX;
    var win_right = win_left + window.innerWidth;
    var elems = document.body.querySelectorAll('a, input:not([type=hidden]), textarea, select, button');
    var div = document.createElement('div');
    div.setAttribute('highlight', 'hints');
    document.body.appendChild(div);
    for (var i = 0; i < elems.length; i++) {
        var elem = elems[i];
        if (!isHintDisplay(elem)) continue;
        var pos = elem.getBoundingClientRect();
        var elem_top = win_top + pos.top;
        var elem_bottom = win_top + pos.bottom;
        var elem_left = win_left + pos.left;
        var elem_right = win_left + pos.left;
        if ( elem_bottom >= win_top && elem_top <= win_bottom) {
            hint_elems.push(elem);
            setHighlight(elem, false);
            var span = document.createElement('span');
            // -------------------------------------------------------------------------- //
            // var json = (function() {
            //     var json = null;
            //         $.ajax({
            //             'async': false,
            //             'global': false,
            //             'url': "/home/mitch/.cache/wal/colors.json",
            //             'dataType': "json",
            //             'success': function (data) {
            //                 json = data;
            //             }
            //         });
            //         return json;
            // })();

            span.style.cssText = [
                'left: ', elem_left, 'px;',
                'top: ', elem_top, 'px;',
                'position: absolute;',
                'font-size: 10px;',
                'background-color:#EE6B6B' + ';',
                'color: #F6F7F5;',
                'padding: 0px 1px;',
                'z-index: 100000;'
                    ].join('');
            // -------------------------------------------------------------------------- //
            span.innerHTML = hint_elems.length;
            div.appendChild(span);
            if (elem.tagName.toLowerCase() == 'a') {
                if (hint_elems.length == 1) setHighlight(elem, true);
                else setHighlight(elem, false);
            }
        }
    }
}

function isHintDisplay(elem) {
    var pos = elem.getBoundingClientRect();
    return (pos.height != 0 && pos.width != 0);
}

function removeHints() {
    if (!hint_enabled) return;
    hint_enabled = false;
    deleteHintRules();
    for (var i = 0; i < hint_elems.length; i++) {
        hint_elems[i].removeAttribute('highlight');
    }
    hint_elems = [];
    hint_num_str = '';
    var div = document.body.querySelector('div[highlight=hints]');
    if (div != undefined) document.body.removeChild(div);
    document.removeEventListener('keydown', hintHandler, false);
    document.addEventListener('keydown', initKeyBind, false);
}

function addKeyBind( key, func, eve ){
    var pressedKey = get_key(eve);
    if( pressedKey == key ){
        eve.preventDefault();  //Stop Default Event
        eval(func);
    }
}

document.addEventListener( 'keydown', initKeyBind, false );

var keyId = {
    "U+0008" : "BackSpace",
    "U+0009" : "Tab",
    "U+0018" : "Cancel",
    "U+001B" : "Esc",
    "U+0020" : "Space",
    "U+0021" : "!",
    "U+0022" : "\"",
    "U+0023" : "#",
    "U+0024" : "$",
    "U+0026" : "&",
    "U+0027" : "'",
    "U+0028" : "(",
    "U+0029" : ")",
    "U+002A" : "*",
    "U+002B" : "+",
    "U+002C" : ",",
    "U+002D" : "-",
    "U+002E" : ".",
    "U+002F" : "/",
    "U+0030" : "0",
    "U+0031" : "1",
    "U+0032" : "2",
    "U+0033" : "3",
    "U+0034" : "4",
    "U+0035" : "5",
    "U+0036" : "6",
    "U+0037" : "7",
    "U+0038" : "8",
    "U+0039" : "9",
    "U+003A" : ":",
    "U+003B" : ";",
    "U+003C" : "<",
    "U+003D" : "=",
    "U+003E" : ">",
    "U+003F" : "?",
    "U+0040" : "@",
    "U+0041" : "a",
    "U+0042" : "b",
    "U+0043" : "c",
    "U+0044" : "d",
    "U+0045" : "e",
    "U+0046" : "f",
    "U+0047" : "g",
    "U+0048" : "h",
    "U+0049" : "i",
    "U+004A" : "j",
    "U+004B" : "k",
    "U+004C" : "l",
    "U+004D" : "m",
    "U+004E" : "n",
    "U+004F" : "o",
    "U+0050" : "p",
    "U+0051" : "q",
    "U+0052" : "r",
    "U+0053" : "s",
    "U+0054" : "t",
    "U+0055" : "u",
    "U+0056" : "v",
    "U+0057" : "w",
    "U+0058" : "x",
    "U+0059" : "y",
    "U+005A" : "z",
    "U+00DB" : "[",
    "U+00DC" : "\\",
    "U+00DD" : "]",
    "U+005E" : "^",
    "U+005F" : "_",
    "U+0060" : "`",
    "U+007B" : "{",
    "U+007C" : "|",
    "U+007D" : "}",
    "U+007F" : "Delete",
    "U+00A1" : "¡",
}

// window.addEventListener("click", function(e) {
//     if ( e.button == 1 || e.ctrlKey ) {
//         var new_uri = e.srcElement.href;
//         if (new_uri) {
//             e.stopPropagation();
//             e.preventDefault();
//             window.open(new_uri);
//         }
//     }
// }, false);

function get_key(evt){
    var key = keyId[evt.keyIdentifier] || evt.keyIdentifier,
        ctrl = evt.ctrlKey ? 'C-' : '',
        meta = (evt.metaKey || evt.altKey) ? 'M-' : '',
        shift = evt.shiftKey ? 'S-' : '';
    if (evt.shiftKey){
        if (/^[a-z]$/.test(key)) return ctrl+meta+key.toUpperCase();
        if (/^[0-9]$/.test(key)) {
            switch(key) {
                case "4": key = "$";
                break;
            }; return key;
        }
        if (/^(Enter|Space|BackSpace|Tab|Esc|Home|End|Left|Right|Up|Down|PageUp|PageDown|F(\d\d?))$/.test(key))
            return ctrl+meta+shift+key;
    }
    return ctrl+meta+key;
}
