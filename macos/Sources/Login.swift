import Foundation

struct LoginLaunch {
    static let label="io.github.rickndanger-ctrl.genesis-agent-controller.login"
    static var file: URL {FileManager.default.homeDirectoryForCurrentUser.appendingPathComponent("Library/LaunchAgents/\(label).plist")}
    static var enabled: Bool {FileManager.default.fileExists(atPath:file.path)}
    static func set(_ enabled: Bool) throws {
        let domain="gui/\(getuid())"
        func run(_ arguments: [String]) -> Int32 {
            let p=Process();p.executableURL=URL(fileURLWithPath:"/bin/launchctl");p.arguments=arguments
            p.standardOutput=FileHandle.nullDevice;p.standardError=FileHandle.nullDevice
            do {try p.run();p.waitUntilExit();return p.terminationStatus} catch {return -1}
        }
        if enabled {
            let app=Bundle.main.bundleURL.path
            let data=try PropertyListSerialization.data(fromPropertyList:["Label":label,"ProgramArguments":["/usr/bin/open","-g","-a",app],"RunAtLoad":true,"LimitLoadToSessionType":"Aqua"],format:.xml,options:0)
            try FileManager.default.createDirectory(at:file.deletingLastPathComponent(),withIntermediateDirectories:true)
            try data.write(to:file,options:.atomic)
            _=run(["bootout",domain+"/"+label]);_ = run(["bootstrap",domain,file.path])
        } else {
            _=run(["bootout",domain+"/"+label])
            if self.enabled {try FileManager.default.removeItem(at:file)}
        }
    }
}
