import Foundation

// Unknown or ambiguous controls must not fabricate a completion.
enum ClaudeActivity {
    static func activeState(headerCount: Int, stopCount: Int, sendCount: Int) -> String? {
        guard headerCount == 1 else { return nil }
        if stopCount == 1 { return "Working" }
        if stopCount == 0 && sendCount == 1 { return "Idle" }
        return nil
    }
}
