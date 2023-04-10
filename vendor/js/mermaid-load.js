import mermaid from "https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs";

/**
 * Alter all ids in the svg adding the index of the graph.
 * Make sure that the style is updated to use the new ids.
 * @param {string} svgString
 * @param {number} idx 
 * @returns {string}
 */
function uniqueIds(svgString, idx) {
    /** @type {Set<string>} */
    const set = new Set();
    svgString = svgString.replace(/<marker.*?id=['"]([^"']+)["'].*?<\/marker>/g, (match, id) => {
        set.add(id);
        return match.replace(id, `${id}-${idx}`);
    });
    if (set.size === 0) return svgString;
    const re = new RegExp(`#(${Array.from(set.values()).join('|')})`, "g");
    return svgString.replace(re, `#$1-${idx}`);
}

function loadMermaid() {
    mermaid.initialize({
        startOnLoad: false,
        theme: "dark",
        flowchart: { htmlLabels: false },
    });
    Reveal.addEventListener("ready", (event) => {
        const graphs = document.getElementsByClassName("mermaid");
        graphs.forEach(async (item, index) => {
            const graphCode = item.textContent.trim();
            const mermaidDiv = document.createElement("div");
            mermaidDiv.classList.add("mermaid");
            mermaidDiv.setAttribute("data-processed", "true");

            try {
                const { svg } = await mermaid.render(`graph-${index}`, graphCode);
                mermaidDiv.innerHTML = uniqueIds(svg, index);
                item.parentNode.parentNode.insertBefore(
                    mermaidDiv,
                    item.parentNode
                );
                item.parentNode.remove();
            } catch (err) {
                console.error(`Cannot render diagram ${index}\n${graphCode}`);
                console.error(err.message);
            }
        });
    });
}

window.loadMermaid = loadMermaid;

