import SwiftUI
import AppKit
import ApplicationServices
import IOKit.hid

// Diagnostic logging is disabled in the public distribution.
func controllerTrace(_ message: String) {}
struct Session: Identifiable { var id: String { title }; let title: String; let state: String }
final class ClaudeAccess {
    let bundle = "com.anthropic.claudefordesktop"
    func value(_ e: AXUIElement, _ key: String) -> AnyObject? { var v: CFTypeRef?; guard AXUIElementCopyAttributeValue(e,key as CFString,&v) == .success else { return nil }; return v }
    func text(_ e: AXUIElement) -> String {
        for k in [kAXTitleAttribute,kAXDescriptionAttribute,kAXValueAttribute] { if let s=value(e,k) as? String, !s.isEmpty { return s } }; return ""
    }
    func nodes() -> [AXUIElement] {
        guard AXIsProcessTrusted(), let app=NSRunningApplication.runningApplications(withBundleIdentifier:bundle).first else { return [] }
        let root=AXUIElementCreateApplication(app.processIdentifier)
        AXUIElementSetMessagingTimeout(root,0.3)
        var queue=[root], result=[AXUIElement](), index=0
        while index<queue.count && index<4500 { let e=queue[index]; index+=1; result.append(e)
            if ["Chat messages","Changes"].contains(text(e)) { continue }
            if let children=value(e,kAXChildrenAttribute) as? [AXUIElement] { queue.append(contentsOf:children) }
        }; return result
    }
    let states=["Awaiting input","Something went wrong","Needs input","Working","Running","Idle","Unread"]
    func sessions(_ nodes: [AXUIElement]) -> [Session] {
        var result=[Session]()
        for e in nodes where value(e,kAXRoleAttribute) as? String == kAXButtonRole {
            let label=text(e)
            for state in states where label.hasPrefix(state+" ") {
                result.append(Session(title:String(label.dropFirst(state.count+1)),state:state));break
            }
        }
        // An open fork may lack a recognized status prefix in the sidebar.
        // The session header is also used to verify navigation and dictation.
        let suffix=", rename session"
        for e in nodes {
            let label=text(e)
            if label.hasSuffix(suffix) {
                let title=String(label.dropLast(suffix.count))
                if !title.isEmpty && !result.contains(where: {$0.title==title}) {
                    result.append(Session(title:title,state:"Open"))
                }
            }
        }
        return result
    }
    func press(_ e: AXUIElement) -> Bool {
        if let enabled=value(e,kAXEnabledAttribute) as? Bool, !enabled { return false }
        return AXUIElementPerformAction(e,kAXPressAction as CFString) == .success
    }
    func current(_ title: String) -> Bool {
        nodes().filter { text($0)==title+", rename session" }.count==1
    }
    // Electron can acknowledge AXPress without activating its DOM button.
    // Only click a freshly resolved, unobscured Claude navigation control.
    @MainActor func navigationClick(_ element: AXUIElement, app: NSRunningApplication) async -> Bool {
        guard NSWorkspace.shared.frontmostApplication?.processIdentifier == app.processIdentifier,
              let pv=value(element,kAXPositionAttribute), let sv=value(element,kAXSizeAttribute),
              CFGetTypeID(pv)==AXValueGetTypeID(), CFGetTypeID(sv)==AXValueGetTypeID() else {return false}
        var point=CGPoint.zero, size=CGSize.zero
        guard AXValueGetValue(pv as! AXValue,.cgPoint,&point), AXValueGetValue(sv as! AXValue,.cgSize,&size), size.width>0,size.height>0 else {return false}
        point.x += size.width/2;point.y += size.height/2
        let previous=CGEvent(source:nil)?.location
        var matched=false
        for attempt in 0..<3 {
            if let position=value(element,kAXPositionAttribute),let dimensions=value(element,kAXSizeAttribute),CFGetTypeID(position)==AXValueGetTypeID(),CFGetTypeID(dimensions)==AXValueGetTypeID() {
                var origin=CGPoint.zero,extent=CGSize.zero
                if AXValueGetValue(position as! AXValue,.cgPoint,&origin),AXValueGetValue(dimensions as! AXValue,.cgSize,&extent),extent.width>0,extent.height>0 {size=extent;point=CGPoint(x:origin.x+extent.width/2,y:origin.y+extent.height/2)}
            }
            var hit: AXUIElement?
            if AXUIElementCopyElementAtPosition(AXUIElementCreateSystemWide(),Float(point.x),Float(point.y),&hit) == .success {
                for _ in 0..<8 {
                    guard let node=hit else {break}
                    var pid:pid_t=0;AXUIElementGetPid(node,&pid)
                    if pid==app.processIdentifier && (CFEqual(node,element) ||
                        (value(node,kAXRoleAttribute) as? String == value(element,kAXRoleAttribute) as? String && text(node)==text(element))) {matched=true;break}
                    hit=value(node,kAXParentAttribute) as! AXUIElement?
                }
            }
            if matched {break}
            controllerTrace("hit check retry=\(attempt)")
            // Dismiss hover overlays and let Electron refresh its hit-test tree.
            CGEvent(mouseEventSource:nil,mouseType:.mouseMoved,mouseCursorPosition:CGPoint(x:point.x+size.width+20,y:point.y),mouseButton:.left)?.post(tap:.cghidEventTap)
            try? await Task.sleep(nanoseconds:180_000_000)
            guard NSWorkspace.shared.frontmostApplication?.processIdentifier == app.processIdentifier else {break}
        }
        guard matched else {
            if let previous {CGEvent(mouseEventSource:nil,mouseType:.mouseMoved,mouseCursorPosition:previous,mouseButton:.left)?.post(tap:.cghidEventTap)}
            return false
        }
        CGEvent(mouseEventSource:nil,mouseType:.leftMouseDown,mouseCursorPosition:point,mouseButton:.left)?.post(tap:.cghidEventTap)
        try? await Task.sleep(nanoseconds:80_000_000)
        CGEvent(mouseEventSource:nil,mouseType:.leftMouseUp,mouseCursorPosition:point,mouseButton:.left)?.post(tap:.cghidEventTap)
        try? await Task.sleep(nanoseconds:80_000_000)
        if let previous {CGEvent(mouseEventSource:nil,mouseType:.mouseMoved,mouseCursorPosition:previous,mouseButton:.left)?.post(tap:.cghidEventTap)}
        return true
    }
    @MainActor func select(_ title: String) async -> String {
        guard let app=NSRunningApplication.runningApplications(withBundleIdentifier:bundle).first else {return "Open Claude first."}
        NSApp.yieldActivation(to: app)
        if let url=app.bundleURL {
            let configuration=NSWorkspace.OpenConfiguration();configuration.activates=true
            _ = try? await NSWorkspace.shared.openApplication(at:url,configuration:configuration)
        } else {app.activate()}
        for _ in 0..<10 {
            if NSWorkspace.shared.frontmostApplication?.processIdentifier == app.processIdentifier {break}
            try? await Task.sleep(nanoseconds:100_000_000)
        }
        guard NSWorkspace.shared.frontmostApplication?.processIdentifier == app.processIdentifier else {return "Claude could not come to the foreground. No click sent."}
        try? await Task.sleep(nanoseconds:200_000_000)
        if current(title) {return "Opened \(title)"}
        let sidebar=nodes().filter {text($0)=="Show sidebar" && value($0,kAXRoleAttribute) as? String == kAXButtonRole}
        if !sessions(nodes()).contains(where: {$0.title==title}), sidebar.count==1 {
            _=await navigationClick(sidebar[0],app:app)
            try? await Task.sleep(nanoseconds:300_000_000)
        }
        let matches=nodes().filter { e in
            guard value(e,kAXRoleAttribute) as? String == kAXButtonRole else {return false}
            let label=text(e)
            if states.contains(where:{label==$0+" "+title}) {return true}
            // Fork/draft rows may use a different status prefix. Require the
            // matching session menu in the same row, not a broad title search.
            guard label==title || label.hasSuffix(" "+title),
                  let parent=value(e,kAXParentAttribute),
                  CFGetTypeID(parent)==AXUIElementGetTypeID(),
                  let siblings=value(parent as! AXUIElement,kAXChildrenAttribute) as? [AXUIElement] else {return false}
            var row=siblings
            var index=0
            while index<row.count && index<40 {
                let node=row[index];index+=1
                if text(node)=="More options for "+title {return true}
                if let children=value(node,kAXChildrenAttribute) as? [AXUIElement] {row.append(contentsOf:children)}
            }
            return false
        }
        guard matches.count==1 else { return "Session missing or ambiguous. Open Claude’s Code sidebar and refresh." }
        guard await navigationClick(matches[0],app:app) else {return "Claude session is obscured or off-screen. No click sent."}
        for _ in 0..<10 {
            try? await Task.sleep(nanoseconds:150_000_000)
            if current(title) {return "Opened \(title)"}
        }
        return "Claude accepted the click but did not open the session. No Send or Dictate action is enabled."
    }
    @MainActor func discover() async -> [Session] {
        guard let app=NSRunningApplication.runningApplications(withBundleIdentifier:bundle).first else {return []}
        NSApp.yieldActivation(to:app)
        if let url=app.bundleURL {let c=NSWorkspace.OpenConfiguration();c.activates=true;_ = try? await NSWorkspace.shared.openApplication(at:url,configuration:c)}
        try? await Task.sleep(nanoseconds:300_000_000)
        if sessions(nodes()).isEmpty {
            let buttons=nodes().filter {text($0)=="Show sidebar" && value($0,kAXRoleAttribute) as? String == kAXButtonRole}
            if buttons.count==1 {_ = press(buttons[0]);try? await Task.sleep(nanoseconds:400_000_000);if sessions(nodes()).isEmpty {_ = await navigationClick(buttons[0],app:app);try? await Task.sleep(nanoseconds:400_000_000)}}
        }
        return sessions(nodes())
    }
    @MainActor func action(_ action: String, title: String) async -> String {
        guard let foregroundApp=NSRunningApplication.runningApplications(withBundleIdentifier:bundle).first else {return "Open Claude first."}
        if NSWorkspace.shared.frontmostApplication?.processIdentifier != foregroundApp.processIdentifier {
            NSApp.yieldActivation(to:foregroundApp)
            if let url=foregroundApp.bundleURL {let c=NSWorkspace.OpenConfiguration();c.activates=true;_ = try? await NSWorkspace.shared.openApplication(at:url,configuration:c)}
            try? await Task.sleep(nanoseconds:250_000_000)
        }
        if !sessions(nodes()).isEmpty {
            let sidebar=nodes().filter {["Show sidebar","Hide sidebar"].contains(text($0)) && value($0,kAXRoleAttribute) as? String == kAXButtonRole}
            if sidebar.count==1 {_ = press(sidebar[0]);try? await Task.sleep(nanoseconds:400_000_000);if !sessions(nodes()).isEmpty {_ = await navigationClick(sidebar[0],app:foregroundApp);try? await Task.sleep(nanoseconds:400_000_000)}}
        }
        let all=nodes()
        guard !title.isEmpty, all.filter({text($0)==title+", rename session"}).count==1 else { return "Select the mapped session first; current session does not match." }
        guard let app=NSRunningApplication.runningApplications(withBundleIdentifier:bundle).first else {return "Open Claude first."}
        let target=action=="send" ? "Send" : (all.contains {text($0)=="Stop dictation"} ? "Stop dictation" : "Dictate")
        controllerTrace("resolve action=\(action) target=\(target)")
        let candidates=all.filter { text($0)==target && [kAXButtonRole,kAXCheckBoxRole,"AXToggleButton","AXSwitch"].contains(value($0,kAXRoleAttribute) as? String ?? "") }
        guard candidates.count==1 else {return action=="dictate" ? "Set Claude Dictation settings → Hold to record OFF, then retry." : "No unique Send button is available."}
        NSApp.yieldActivation(to: app)
        app.activate()
        if action=="dictate" {
            let wasRecording=target=="Stop dictation"
            func recordingState() -> Bool? {
                let controls=nodes().filter { ["Dictate","Stop dictation"].contains(text($0)) && [kAXButtonRole,kAXCheckBoxRole,"AXToggleButton","AXSwitch"].contains(value($0,kAXRoleAttribute) as? String ?? "") }
                guard controls.count==1 else {return nil}
                let recording=text(controls[0])=="Stop dictation"
                controllerTrace("observed recording=\(recording)")
                return recording
            }
            guard recordingState()==wasRecording, current(title) else {return "Could not confirm microphone state. No click sent."}
            let fresh=nodes().filter {text($0)==target && [kAXButtonRole,kAXCheckBoxRole,"AXToggleButton","AXSwitch"].contains(value($0,kAXRoleAttribute) as? String ?? "")}
            guard fresh.count==1, await navigationClick(fresh[0],app:app) else {return "Microphone control is unavailable."}
            for _ in 0..<10 {
                try? await Task.sleep(nanoseconds:100_000_000)
                if recordingState() == !wasRecording {return wasRecording ? "Dictation stopped" : "Dictation started"}
            }
            return "Microphone did not change state. Try again."
        }
        return await navigationClick(candidates[0],app:app) ? "Sent composer" : "Control is disabled or unavailable."
    }
}

final class HIDLink {
    var manager: IOHIDManager!; var device: IOHIDDevice?
    let buffer=UnsafeMutablePointer<UInt8>.allocate(capacity:128)
    var bytes=Data(); var nextID=40000
    var received: (([String:Any])->Void)?
    func start() {
        manager=IOHIDManagerCreate(kCFAllocatorDefault,IOOptionBits(kIOHIDOptionsTypeNone))
        IOHIDManagerSetDeviceMatching(manager,[kIOHIDVendorIDKey:0x303a,kIOHIDProductIDKey:0x8360] as CFDictionary)
        let context=Unmanaged.passUnretained(self).toOpaque()
        IOHIDManagerRegisterDeviceMatchingCallback(manager,{ context,_,_,dev in
            guard let context else{return};let me=Unmanaged<HIDLink>.fromOpaque(context).takeUnretainedValue()
            guard let serial=IOHIDDeviceGetProperty(dev,kIOHIDSerialNumberKey as CFString) as? String,
                  serial.range(of:"^GENESIS-[0-9A-F]{12}$",options:.regularExpression) != nil, me.device == nil else{return}
            guard IOHIDDeviceOpen(dev,0)==kIOReturnSuccess else{return};me.device=dev
            IOHIDDeviceRegisterInputReportCallback(dev,me.buffer,128,{ctx,_,_,_,_,report,length in
                guard let ctx else{return};Unmanaged<HIDLink>.fromOpaque(ctx).takeUnretainedValue().input(Data(bytes:report,count:length))
            },context)
        },context)
        IOHIDManagerRegisterDeviceRemovalCallback(manager,{context,_,_,removed in
            guard let context else{return};let me=Unmanaged<HIDLink>.fromOpaque(context).takeUnretainedValue();guard let connected=me.device,CFEqual(connected,removed) else{return};me.device=nil;me.bytes.removeAll();me.received?(["disconnected":true])
        },context)
        IOHIDManagerScheduleWithRunLoop(manager,CFRunLoopGetMain(),CFRunLoopMode.defaultMode.rawValue)
        IOHIDManagerOpen(manager,0)
    }
    func input(_ report: Data) {
        var r=[UInt8](report);if r.count==64 && r[0]==6 {r.removeFirst()}
        guard r.count>=2,r[0]==2,Int(r[1])<=61,r.count>=Int(r[1])+2 else{return}
        bytes.append(contentsOf:r[2..<(Int(r[1])+2)])
        if bytes.count>16384 {bytes.removeAll();return}
        while let end=bytes.firstIndex(of:10) {
            let line=bytes.prefix(upTo:end);bytes.removeSubrange(...end)
            if let object=try? JSONSerialization.jsonObject(with:line) as? [String:Any] {received?(object)}
        }
    }
    func send(_ method: String,_ params: Any) {
        guard let device else{return};nextID+=1
        guard let data=try? JSONSerialization.data(withJSONObject:["method":method,"params":params,"id":nextID]) else{return}
        let raw=[UInt8](data)
        for start in stride(from:0,to:raw.count,by:61) {
            let count=min(61,raw.count-start);var report=[UInt8](repeating:0,count:64)
            report[0]=6;report[1]=2;report[2]=UInt8(count);report.replaceSubrange(3..<3+count,with:raw[start..<start+count])
            let status=report.withUnsafeBufferPointer {IOHIDDeviceSetReport(device,kIOHIDReportTypeOutput,6,$0.baseAddress!,64)}
            if status != kIOReturnSuccess {return}
        }
    }
}

@MainActor final class Model: ObservableObject {
    @Published var sessions=[Session]()
    @Published var knownTitles=UserDefaults.standard.stringArray(forKey:"knownSessionTitles") ?? [String]()
    @Published var slots: [String] = (UserDefaults.standard.stringArray(forKey:"slots") ?? Array(repeating:"",count:6)) { didSet {UserDefaults.standard.set(slots,forKey:"slots")} }
    @Published var slotActions: [String] = UserDefaults.standard.stringArray(forKey:"slotActions") ?? Array(repeating:"chat",count:6) {didSet {UserDefaults.standard.set(slotActions,forKey:"slotActions")}}
    func slotLabel(_ i:Int)->String {switch slotActions[i] {case "dictate":return "Hold to talk";case "send":return "Send prompt";default:return slots[i].isEmpty ? "Assign chat" : slots[i]}}
    @Published var mode="Waiting for Genesis"
    @Published var status="Open Claude’s Code sidebar, then Refresh sessions."
    @Published var trusted=false
    @Published var selected = -1
    @Published var selecting=false
    @Published var loginEnabled=LoginLaunch.enabled
    @Published var soundEnabled=true
    @Published var encoderDetents=1
    @Published var firmware="Not connected"
    @Published var talkAfterSelect=UserDefaults.standard.object(forKey:"talkAfterSelect") as? Bool ?? true {didSet {UserDefaults.standard.set(talkAfterSelect,forKey:"talkAfterSelect");configure(["talk_after_select":talkAfterSelect])}}
    @Published var microphoneAction=UserDefaults.standard.string(forKey:"microphoneAction") ?? "dictate" {didSet {UserDefaults.standard.set(microphoneAction,forKey:"microphoneAction")}}
    @Published var sendAction=UserDefaults.standard.string(forKey:"sendAction") ?? "send" {didSet {UserDefaults.standard.set(sendAction,forKey:"sendAction")}}
    var supportsSettings: Bool {firmware != "Not connected" && firmware.compare("v0.2.2",options:.numeric) != .orderedAscending}
    func configure(_ params:[String:Any]) {guard supportsSettings else{return};hid.send("genesis.settings",params);hid.send("device.status",[:])}
    func setLogin(_ enabled:Bool) {do {try LoginLaunch.set(enabled);loginEnabled=LoginLaunch.enabled} catch {status="Could not update login startup: \(error.localizedDescription)"}}
    func slotState(_ i:Int)->String {if slotActions[i] != "chat" {return "Shortcut"};let matches=sessions.filter{$0.title==slots[i]};return slots[i].isEmpty ? "Unassigned" : matches.count==1 ? matches[0].state : "Not visible"}

    var actionBusy=false
    var pendingDictation=false
    var selectionGeneration=0
    let ax=ClaudeAccess();let hid=HIDLink();var timer:Timer?;var lastReply=Date.distantPast
    init() {
        if slotActions.count != 6 {slotActions=Array(repeating:"chat",count:6)}
        if slots.count != 6 {slots=Array(repeating:"",count:6)}
        hid.received={ [weak self] message in Task { @MainActor in self?.receive(message) } };hid.start()
        timer=Timer.scheduledTimer(withTimeInterval:2,repeats:true) { [weak self] _ in Task { @MainActor in self?.tick() } }
        refresh()
    }
    func refresh() {trusted=AXIsProcessTrusted();if trusted {sessions=ax.sessions(ax.nodes());rememberSessions()};if !trusted {status="Enable Accessibility for Genesis Agent Micro Controller to read Claude controls."}}
    func rememberSessions() {
        knownTitles=Array(Set(knownTitles+sessions.map(\.title)+slots.filter{!$0.isEmpty})).sorted()
        UserDefaults.standard.set(knownTitles,forKey:"knownSessionTitles")
    }
    func discoverSessions() {
        Task { @MainActor in
            sessions=await ax.discover();rememberSessions()
            status=sessions.isEmpty ? "No chats found. Open Claude’s Code tab and try Refresh sessions." : "Found \(sessions.count) chats. Choose a key and its Claude session."
            NSApp.activate()
        }
    }
    func tick() {
        if hid.device==nil {mode="Genesis disconnected";return}
        hid.send("device.status",[:])
        hid.send("genesis.settings",["codex_chat_mask":CodexSlotTypes.currentMask()])
        if Date().timeIntervalSince(lastReply)>6 {mode="Waiting for firmware reply"}
        if mode=="Claude" { refresh();let entries=(0..<6).map { i -> [String:Any] in
            let matches=sessions.filter{$0.title==slots[i]};let state=matches.count==1 ? matches[0].state : ""
            let color: Int
            switch state {case "Awaiting input","Needs input":color=0xff6d00;case "Something went wrong":color=0xff0033;case "Working","Running":color=0x304ffe;case "Unread":color=0x00ff4c;case "Idle":color=0xffffff;default:color=0}
            return ["id":i,"c":color,"b":1]
        };hid.send("genesis.claude.status",entries)}
    }
    func selectSlot(_ slot: Int) {
        guard !selecting,mode=="Claude" else {return}
        if slotActions[slot] != "chat" {runAction(slotActions[slot]);return}
        guard !slots[slot].isEmpty else {status="Assign C\(slot+1) first.";return}
        selected = -1;selecting=true;selectionGeneration+=1
        let generation=selectionGeneration,title=slots[slot]
        status="Opening \(title)…"
        Task { @MainActor in
            let result=await ax.select(title)
            selecting=false
            guard generation==selectionGeneration,mode=="Claude",slots[slot]==title else {return}
            status=result;selected=result.hasPrefix("Opened ") ? slot : -1
            if selected==slot && talkAfterSelect {configure(["focus_dictate":true])}
        }
    }
    func runAction(_ action:String) {
        controllerTrace("request action=\(action) busy=\(actionBusy)")
        if actionBusy {
            if action=="dictate" {pendingDictation.toggle();status="Microphone change queued"}
            return
        }
        let headers=ax.nodes().map{ax.text($0)}.filter{$0.hasSuffix(", rename session")}
        guard headers.count==1 else {status="Open a Claude Code chat first.";return}
        let title=String(headers[0].dropLast(", rename session".count))
        actionBusy=true
        Task { @MainActor in
            status=await ax.action(action,title:title)
            controllerTrace("result=\(status)")
            while pendingDictation {
                pendingDictation=false
                guard mode=="Claude",ax.current(title) else {break}
                status=await ax.action("dictate",title:title)
            }
            actionBusy=false
        }
    }
    func receive(_ msg: [String:Any]) {
        if msg["disconnected"] != nil {mode="Genesis disconnected";firmware="Not connected";selected = -1;selectionGeneration+=1;return}
        if let result=msg["result"] as? [String:Any],result["version"] != nil {
            let firstReply=firmware=="Not connected"
            firmware=result["version"] as? String ?? "Not connected"
            soundEnabled=result["sound_enabled"] as? Bool ?? soundEnabled
            encoderDetents=result["encoder_detents"] as? Int ?? encoderDetents
            if firstReply {configure(["talk_after_select":talkAfterSelect])}
            lastReply=Date();let updated=(result["controller_mode"] as? String)=="claude" ? "Claude" : "Codex"
            if updated != mode {selected = -1;selectionGeneration+=1};mode=updated
            if result["controller_mode"] == nil {status="Working Codex firmware detected. Dual-mode update is not installed."}
        }
        guard msg["m"] as? String == "genesis.input",let p=msg["p"] as? [String:Any],p["act"] as? Int==1 else{return}
        let key=p["k"] as? String ?? ""
        controllerTrace("HID key=\(key)")
        if key=="MODE" {mode=(p["mode"] as? String)=="claude" ? "Claude" : "Codex";selected = -1;selectionGeneration+=1;return}
        guard mode=="Claude" else{return}
        if let slot=p["ag"] as? Int,(0..<6).contains(slot) {
            selectSlot(slot);return
        }
        let action=key=="DICTATE" ? microphoneAction : key=="SEND" ? sendAction : "none"
        if action != "none" {runAction(action)}
    }
}
