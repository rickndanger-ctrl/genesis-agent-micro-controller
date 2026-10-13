import Foundation

// Read only the slot-kind metadata needed for navigation. Never modify Codex settings.
enum CodexSlotTypes {
    static func mask(config: String, state: Data) -> Int {
        var section = "", source: String?
        for raw in config.components(separatedBy: .newlines) {
            let line = raw.trimmingCharacters(in: .whitespaces)
            if line.hasPrefix("[") { section = line; continue }
            guard section == "[desktop]" else { continue }
            let pattern = #"^codex-micro-agent-source\s*=\s*["'](custom|recent|pinned|priority)["']\s*(?:#.*)?$"#
            if let match = line.range(of: pattern, options: .regularExpression) {
                let value = String(line[match])
                source = ["custom", "recent", "pinned", "priority"].first { value.contains("\"\($0)\"") || value.contains("'\($0)'") }
            }
        }
        guard let source else { return 0 } // Unknown configuration must not force microphone navigation.
        if source != "custom" { return 63 }
        guard let root = (try? JSONSerialization.jsonObject(with: state)) as? [String: Any],
              let atoms = root["electron-persisted-atom-state"] as? [String: Any],
              let assignments = atoms["codex-micro-custom-agent-assignments"] as? [String: Any] else { return 0 }
        return (0..<6).reduce(0) { mask, slot in
            let entry = assignments[String(format: "AG%02d", slot)] as? [String: Any]
            let thread = entry?["threadKey"] as? String
            return entry?["type"] == nil && thread?.isEmpty == false ? mask | (1 << slot) : mask
        }
    }
    static func currentMask() -> Int {
        let home = ProcessInfo.processInfo.environment["CODEX_HOME"].map { URL(fileURLWithPath: $0) }
            ?? FileManager.default.homeDirectoryForCurrentUser.appendingPathComponent(".codex")
        guard let config = try? String(contentsOf: home.appendingPathComponent("config.toml"), encoding: .utf8),
              let state = try? Data(contentsOf: home.appendingPathComponent(".codex-global-state.json")) else { return 0 }
        return mask(config: config, state: state)
    }
}
