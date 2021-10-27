const KeyMap = require("./keymap.js");

module.exports = class KeycodeParser {
  match_index = 0;
  match_keyobj = {};

  constructor() { }
  parse = (obj, cb) => {
    let json = {};

    if (obj.type == "keydown") {
      const keycode = obj.keycode - 0;
      const ctrl = obj.ctrl ? true : false;
      const shift = obj.shift ? true : false;
      const alt = obj.alt ? true : false;
      const contno = obj.contno;
      const tabno = obj.tabno;
      const mode = obj.mode;


      let keymapobj = KeyMap;
      if (0 < this.match_index) {
        keymapobj = this.match_keyobj;
      }
      let mobj = {};
      let mflg = false;

      for (let objkey in keymapobj) {
        let obj = KeyMap[objkey];
        if (!obj.keys || obj.keys.length < 1)
          continue;

        for (const keyary of obj.keys) {
          let keylen = keyary.length;
          let keyobj = keyary[this.match_index];

          if (keyobj && keyobj.keycode === keycode && keyobj.ctrl === ctrl && keyobj.shift === shift && keyobj.alt === alt) {
            if (keylen-1 === this.match_index) {
              mobj = {};
              this.clear_match();
              obj.json.contno = contno;
              obj.json.tabno = tabno;
              obj.json.mode = mode;


              console.log(obj.json);
              cb(obj.json);
              return;
            } else {
              mflg = true;
              mobj[objkey] = obj;
            }
          }
          continue;
        }
      }

      if (mflg) {
        this.match_index++;
        this.match_keyobj = mobj;
      }else{
        this.clear_match();
      }

      console.log("");
      console.log(this.match_index);
      console.log(this.match_keyobj);


      json.type = "none";
      cb(json);
      console.log(json);
    } else {
      json.type = "none";
      cb(json);
      console.log(json);
    }
  }
  clear_match = () => {
    this.match_index = 0;
    this.match_keyobj = {};
  }
}
