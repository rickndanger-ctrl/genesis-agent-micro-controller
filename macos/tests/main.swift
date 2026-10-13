import Foundation
let data = #"{"electron-persisted-atom-state":{"codex-micro-custom-agent-assignments":{"AG00":{"threadKey":"thread-one"},"AG01":{"type":"command","commandId":"composer.startVoiceMode"},"AG02":null,"AG03":{"threadKey":""},"AG04":{"type":"skill","skillName":"example"},"AG05":{"threadKey":"thread-six"}}}}"#.data(using:.utf8)!
assert(CodexSlotTypes.mask(config:"[desktop]\ncodex-micro-agent-source = \"custom\"",state:data)==33)
assert(CodexSlotTypes.mask(config:"[desktop]\ncodex-micro-agent-source = 'recent' # test",state:data)==63)
assert(CodexSlotTypes.mask(config:"[other]\ncodex-micro-agent-source = \"custom\"",state:data)==0)
assert(CodexSlotTypes.mask(config:"",state:data)==0)
assert(CodexSlotTypes.mask(config:"[desktop]\ncodex-micro-agent-source = \"custom\"",state:Data())==0)
print("Slot-kind tests passed; current chat mask:",CodexSlotTypes.currentMask())
