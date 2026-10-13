import SwiftUI
import AppKit

struct MainView: View {
 @ObservedObject var model: Model
 @State private var selectedKey=0
 @State private var section=0
 private let accent=Color(red:0.61,green:0.36,blue:0.96)
 private let keyColors:[Color]=[0x3478f6,0x9b5cf6,0xf0529c,0xff9933,0x30c778,0x29c4de].map { rgb in Color(red:Double((rgb>>16)&255)/255,green:Double((rgb>>8)&255)/255,blue:Double(rgb&255)/255) }
 private let panel=Color(red:0.065,green:0.085,blue:0.12)
 func stateColor(_ state:String)->Color {
  switch state {case "Working","Running":return .blue;case "Awaiting input","Needs input":return .orange;case "Something went wrong":return .red;case "Unread":return .green;case "Idle":return .purple;default:return .gray}
 }
 var body: some View {
  HStack(spacing:0) {
   VStack(alignment:.leading,spacing:10) {
    Image(nsImage:NSImage(contentsOf:Bundle.main.url(forResource:"Genesis",withExtension:"png")!) ?? NSImage()).resizable().scaledToFit().frame(width:76,height:76).clipShape(RoundedRectangle(cornerRadius:18)).padding(.bottom,8)
    Text("GENESIS").font(.system(size:23,weight:.bold,design:.rounded)).lineSpacing(3)
    Text("AGENT MICRO\nCONTROLLER").font(.system(size:9,weight:.semibold)).tracking(2).foregroundStyle(.secondary).padding(.bottom,24)
    nav("Agent keys",icon:"square.grid.3x2",index:0)
    nav("Actions",icon:"waveform",index:1)
    nav("Device",icon:"slider.horizontal.3",index:2)
    Spacer()
    HStack(spacing:7) {Circle().fill(model.mode=="Claude" || model.mode=="Codex" ? Color.green:Color.gray).frame(width:7,height:7);Text(model.mode).font(.caption)}
    Text("CLAUDE + CODEX").font(.caption2).foregroundStyle(.secondary)
   }.padding(24).frame(width:190,alignment:.leading).background(panel)
   VStack(alignment:.leading,spacing:20) {
    HStack(alignment:.top) {
     VStack(alignment:.leading,spacing:5) {Text(section==0 ? "Agent keys":section==1 ? "Actions":"Device settings").font(.system(size:26,weight:.semibold));Text(section==0 ? "Six keys. Your agents. One dial.":section==1 ? "Make the next press yours.":"Your controller, ready when you are.").foregroundStyle(.secondary)}
     Spacer()
     Text(model.mode=="Claude" ? "CLAUDE MODE":model.mode=="Codex" ? "CODEX MODE":"OFFLINE").font(.system(size:10,weight:.semibold)).padding(.horizontal,12).padding(.vertical,7).background(accent.opacity(0.12)).clipShape(Capsule())
    }
    if !model.trusted {
     HStack {Image(systemName:"hand.raised.fill");Text("Allow Genesis Agent Micro Controller to use Claude’s controls.");Spacer();Button("Enable access"){NSWorkspace.shared.open(URL(string:"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility")!)}}.font(.callout).padding(12).background(Color.orange.opacity(0.12)).cornerRadius(10)
    }
    if section==0 {agentPage}
    else if section==1 {actionsPage}
    else {devicePage}
    Spacer(minLength:0)
    HStack(alignment:.top,spacing:9) {Image(systemName:"info.circle").foregroundStyle(accent);Text(model.status).font(.callout).textSelection(.enabled);Spacer()}.padding(14).frame(maxWidth:.infinity,alignment:.leading).background(panel).cornerRadius(12)
   }.padding(28).frame(maxWidth:.infinity,maxHeight:.infinity,alignment:.topLeading)
  }.frame(minWidth:980,minHeight:700).background(LinearGradient(colors:[Color(red:0.035,green:0.045,blue:0.07),Color(red:0.065,green:0.08,blue:0.12)],startPoint:.topLeading,endPoint:.bottomTrailing)).tint(accent).preferredColorScheme(.dark)
 }
 func nav(_ title:String,icon:String,index:Int)->some View {
  Button {section=index} label:{Label(title,systemImage:icon).font(.system(size:13,weight:section==index ? .semibold:.regular)).frame(maxWidth:.infinity,alignment:.leading).padding(11).background(section==index ? accent.opacity(0.14):Color.clear).cornerRadius(9)}.buttonStyle(.plain)
 }
 var agentPage: some View {
  HStack(alignment:.top,spacing:24) {
   VStack(spacing:18) {
    HStack {Circle().fill(.green.opacity(model.mode=="Claude" ? 1:0.2)).frame(width:6,height:6);Text("GENESIS  /  CONTROL DECK").font(.system(size:9,weight:.bold,design:.monospaced)).tracking(1);Spacer();Image(systemName:"cable.connector").foregroundStyle(.secondary)}
    LazyVGrid(columns:Array(repeating:GridItem(.flexible(),spacing:10),count:3),spacing:10) {
     ForEach(0..<6,id:\.self) {i in
      Button {selectedKey=i} label:{VStack(spacing:9) {HStack {Circle().fill(keyColors[i]).frame(width:7,height:7);Spacer();Text("C\(i+1)").font(.system(size:11,weight:.bold,design:.monospaced))};Text(model.slotLabel(i)).font(.system(size:11,weight:.medium)).lineLimit(2).frame(maxWidth:.infinity,minHeight:30,alignment:.leading)}.padding(11).frame(height:94).background(keyColors[i].opacity(selectedKey==i ? 0.25:0.09)).cornerRadius(11).overlay(RoundedRectangle(cornerRadius:11).stroke(keyColors[i].opacity(selectedKey==i ? 1:0.35),lineWidth:selectedKey==i ? 2:1))}.buttonStyle(.plain).accessibilityLabel("Configure C\(i+1)")
     }
    }
    HStack {
     VStack(alignment:.leading,spacing:6) {Text("TURN TO CHOOSE").font(.system(size:9,weight:.semibold)).tracking(1);Text("Click to act · double-click back").font(.caption).foregroundStyle(.secondary)}
     Spacer()
     ZStack {Circle().fill(Color(red:0.035,green:0.045,blue:0.07)).frame(width:72,height:72).shadow(color:.black.opacity(0.12),radius:3,y:2);Circle().stroke(accent.opacity(0.4),lineWidth:2).frame(width:58,height:58);Capsule().fill(accent).frame(width:3,height:10).offset(y:-22);Image(systemName:"arrow.trianglehead.2.clockwise.rotate.90").foregroundStyle(.secondary)}
    }
    HStack {Label("Hold to talk",systemImage:"mic");Spacer();Label("Send",systemImage:"paperplane")}.font(.caption).foregroundStyle(.secondary)
   }.padding(20).frame(width:355).background(panel).cornerRadius(23).overlay(RoundedRectangle(cornerRadius:23).stroke(.primary.opacity(0.08)))
   VStack(alignment:.leading,spacing:18) {
    HStack {Text("C\(selectedKey+1)").font(.system(size:42,weight:.bold,design:.rounded));Spacer();Text("KEY CONFIGURATION").font(.system(size:9,weight:.bold,design:.monospaced)).tracking(1.4).foregroundStyle(accent)}
    Text("Make it yours").font(.headline)
    Picker("Assignment type",selection:$model.slotActions[selectedKey]) {
     Text("Chat").tag("chat")
     Text("Hold to talk").tag("dictate")
     Text("Send prompt").tag("send")
    }
    if model.slotActions[selectedKey]=="chat" {
    Picker("Claude session",selection:$model.slots[selectedKey]) {
     Text("Unassigned").tag("")
     if !model.slots[selectedKey].isEmpty && !model.knownTitles.contains(model.slots[selectedKey]) {Text(model.slots[selectedKey]+" (not visible)").tag(model.slots[selectedKey])}
     ForEach(model.knownTitles,id:\.self){Text($0).tag($0)}
    }.labelsHidden().frame(maxWidth:.infinity)
    }
    Label(model.slotState(selectedKey),systemImage:"circle.fill").font(.caption).foregroundStyle(keyColors[selectedKey])
    Button(model.slotActions[selectedKey]=="chat" ? "Open C\(selectedKey+1)" : "Run C\(selectedKey+1)"){model.selectSlot(selectedKey)}.buttonStyle(.borderedProminent).disabled(model.mode != "Claude" || (model.slotActions[selectedKey]=="chat" && model.slots[selectedKey].isEmpty) || model.selecting)
    Button("Refresh sessions",systemImage:"arrow.clockwise"){model.discoverSessions()}
    Divider()
    Text("Refresh sessions discovers Claude chats. Saved choices remain available when the sidebar is closed.").font(.caption).foregroundStyle(.secondary)
    Text("Middle button: switch Claude / Codex. Double-click the dial: return to chats.").font(.caption).foregroundStyle(.secondary)
   }.frame(maxWidth:.infinity,alignment:.leading)
  }
 }
 var actionsPage: some View {
  VStack(alignment:.leading,spacing:22) {
   GroupBox {VStack(alignment:.leading,spacing:18) {
    actionPicker("Microphone key",icon:"mic.fill",selection:$model.microphoneAction)
    Divider()
    actionPicker("Send key",icon:"paperplane.fill",selection:$model.sendAction)
   }.padding(12)}
   GroupBox {VStack(alignment:.leading,spacing:8) {Toggle("After opening a chat, jump to Hold to talk",isOn:$model.talkAfterSelect);Text("The controller selects the microphone action. Press the knob when you’re ready to record; recording never starts automatically.").font(.caption).foregroundStyle(.secondary)}.padding(12)}
   Text("Claude dictation uses click to start and click to stop. Keep Hold to record off in Claude’s dictation settings. Send uses the text already in the selected chat.").font(.callout).foregroundStyle(.secondary)
   Text("Approval controls are not available in this version.").font(.caption).foregroundStyle(.secondary)
  }
 }
 func actionPicker(_ title:String,icon:String,selection:Binding<String>)->some View {
  HStack {Label(title,systemImage:icon).font(.headline);Spacer();Picker(title,selection:selection){Text("Dictation").tag("dictate");Text("Send prompt").tag("send");Text("Do nothing").tag("none")}.labelsHidden().frame(width:170)}
 }
 var devicePage: some View {
  VStack(alignment:.leading,spacing:20) {
   GroupBox {VStack(spacing:18) {
    Toggle("Launch Genesis Agent Micro Controller when I sign in",isOn:Binding(get:{model.loginEnabled},set:{model.setLogin($0)}))
    Divider()
    Toggle("Alert sound",isOn:Binding(get:{model.soundEnabled},set:{model.configure(["sound_enabled":$0])})).disabled(!model.supportsSettings)
    HStack {Text("Knob movement");Spacer();Picker("Knob movement",selection:Binding(get:{model.encoderDetents},set:{model.configure(["encoder_detents":$0])})){Text("Every click").tag(1);Text("Every two clicks").tag(2)}.labelsHidden().frame(width:185).disabled(!model.supportsSettings)}
    HStack {Button("Test light and sound"){model.configure(["test_alert":true])}.disabled(!model.supportsSettings || model.mode != "Claude");Spacer();Text(model.firmware).font(.caption.monospaced()).foregroundStyle(.secondary)}
   }.padding(12)}
   Text("Close this window to keep the controller running. Reopen settings from its menu-bar dial icon.").font(.callout).foregroundStyle(.secondary)
   if !model.supportsSettings {Text("The new knob and device settings need firmware v0.2.2 or later.").font(.caption).foregroundStyle(.orange)}
   Text("Built for your Genesis Mini · independent companion for Claude Code").font(.caption).foregroundStyle(.secondary)
  }
 }
}
@main struct MapperApp: App {
 @StateObject private var model=Model()
 var body: some Scene {
  WindowGroup("Genesis Agent Micro Controller",id:"controller"){MainView(model:model)}.defaultSize(width:1040,height:740)
  MenuBarExtra("Genesis Agent Micro Controller",systemImage:"dial.medium.fill") {ControllerMenu(model:model)}
 }
}
struct ControllerMenu: View {
 @ObservedObject var model:Model
 @Environment(\.openWindow) var openWindow
 var body: some View {Text(model.mode);Button("Open Genesis Agent Micro Controller"){openWindow(id:"controller");NSApp.activate()};Button("Refresh sessions"){model.discoverSessions()};Divider();Button("Quit Genesis Agent Micro Controller"){NSApp.terminate(nil)}}
}
