
const VK_LEFT = 0x25
const VK_UP = 0x26
const VK_RIGHT = 0x27
const VK_DOWN = 0x28
const VK_BACK = 0x08
const VK_DELETE = 0x2E
const VK_A = 0x41
const VK_B = 0x42
const VK_C = 0x43
const VK_D = 0x44
const VK_E = 0x45
const VK_F = 0x46
const VK_G = 0x47
const VK_H = 0x48
const VK_I = 0x49
const VK_J = 0x4A
const VK_K = 0x4B
const VK_L = 0x4C
const VK_M = 0x4D
const VK_N = 0x4E
const VK_O = 0x4F
const VK_P = 0x50
const VK_Q = 0x51
const VK_R = 0x52
const VK_S = 0x53
const VK_T = 0x54
const VK_U = 0x55
const VK_V = 0x56
const VK_W = 0x57
const VK_X = 0x58
const VK_Y = 0x59
const VK_Z = 0x5A

module.exports = {
  "caret_left": {
    keys: [
      [
        {
          keycode: VK_LEFT,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "caret",
          command: "left"
        }
      ]
    }
  },
//////////////////////////////////////////////////combination
  "caret_left_up": {
    keys: [
      [
        {
          keycode: VK_LEFT,
          ctrl: true,
          shift: false,
          alt: false
        },
        {
          keycode: VK_UP,
          ctrl: true,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "caret",
          command: "left"
        },
        {
          type: "caret",
          command: "up"
        }
      ]
    }
  },
//////////////////////////////////////////////////
  "caret_right": {
    keys: [
      [
        {
          keycode: VK_RIGHT,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "caret",
          command: "right"
        }
      ]
    }
  },
  "caret_up": {
    keys: [
      [
        {
          keycode: VK_UP,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "caret",
          command: "up"
        }
      ]
    }
  },
  "caret_down": {
    keys: [
      [
        {
          keycode: VK_DOWN,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "caret",
          command: "down"
        }
      ]
    }
  },
  "backspace": {
    keys: [
      [
        {
          keycode: VK_BACK,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "input",
          command: "backspace"
        }
      ]
    }
  },
  "delete": {
    keys: [
      [
        {
          keycode: VK_DELETE,
          ctrl: false,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "input",
          command: "delete"
        }
      ]
    }
  },
  "clipboard_paste": {
    keys: [
      [
        {
          keycode: VK_V,
          ctrl: true,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "clipboard",
          command: "paste"
        }
      ]
    }
  },
  "clipboard_copy": {
    keys: [
      [
        {
          keycode: VK_C,
          ctrl: true,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "clipboard",
          command: "copy"
        }
      ]
    }
  },
  "clipboard_cut": {
    keys: [
      [
        {
          keycode: VK_X,
          ctrl: true,
          shift: false,
          alt: false
        }
      ]
    ],
    json: {
      commands: [
        {
          type: "clipboard",
          command: "cut"
        }
      ]
    }
  }

};