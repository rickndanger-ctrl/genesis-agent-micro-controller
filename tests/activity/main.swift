import Foundation
assert(ClaudeActivity.activeState(headerCount:1,stopCount:1,sendCount:0)=="Working")
assert(ClaudeActivity.activeState(headerCount:1,stopCount:0,sendCount:1)=="Idle")
assert(ClaudeActivity.activeState(headerCount:1,stopCount:0,sendCount:0)==nil)
assert(ClaudeActivity.activeState(headerCount:2,stopCount:1,sendCount:0)==nil)
assert(ClaudeActivity.activeState(headerCount:1,stopCount:2,sendCount:1)==nil)
assert(ClaudeActivity.activeState(headerCount:1,stopCount:1,sendCount:1)=="Working")
print("Activity state checks passed")
