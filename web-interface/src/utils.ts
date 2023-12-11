export default function log(msg: any) {
    let line = document.createElement("p");
    line.textContent = msg;
    document.getElementById("console")?.appendChild(line);
    console.log(msg);
}