export default function log(msg: any) {
    console.log(msg);
    let consoleDiv = document.getElementById("console");
    if (consoleDiv) {
        let line = document.createElement("span");
        line.textContent = "> " +msg;
        consoleDiv.appendChild(document.createElement("br"));
        consoleDiv.appendChild(line);
        consoleDiv.scrollTop = consoleDiv.scrollHeight;
    }
}