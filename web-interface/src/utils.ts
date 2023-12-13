function getTimestamp(): string {
    let d: Date = new Date();
    let s: string = d.getHours() + ":" + d.getMinutes() + ":" + d.getSeconds() + "." + d.getMilliseconds();
    return s;
}

export default function log(msg: any) {
    console.log(msg);
    let consoleDiv = document.getElementById("console");
    if (consoleDiv) {
        let line = document.createElement("span");
        line.textContent = getTimestamp() + " > " +msg;
        consoleDiv.appendChild(document.createElement("br"));
        consoleDiv.appendChild(line);
        consoleDiv.scrollTop = consoleDiv.scrollHeight;
    }
}