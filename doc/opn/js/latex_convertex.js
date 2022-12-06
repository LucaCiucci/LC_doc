
/**
 * @typedef {Object} RemoteFile
 * @property {string} url
 * @property {string} relPath
 */

/**
 * @typedef {Object} ConversionContext
 * @property {RemoteFile[]} remoteFiles
 * @property {Set<string>} packages
 */

function addPackage(ctx, name) {
    ctx.packages.add(`\\usepackage{${name}}`);
}

function resolveUrl(url) {
    let a = document.createElement("a");
    a.href = url;
    return a.href;
}

/**
 * 
 * @param {string} path
 * @returns {string}
 */
function parentPath(path) {
    return path.substring(0, path.lastIndexOf("/"));
}

/**
 * @typedef {(element : HTMLElement, ctx : ConversionContext) => string} Handler
 */

/**
 * @type {{ [key: string]: Handler }}
 */
var handlers = {
}

/**
 * 
 * @param {Node} node
 * @returns {boolean}
 */
function isTextNode(node) {
    return node.nodeType === Node.TEXT_NODE;
}

function isElementNode(node) {
    return node.nodeType === Node.ELEMENT_NODE;
}

function isCommentNode(node) {
    return node.nodeType === Node.COMMENT_NODE;
}

/**
 * @param {string} url
 */
function isRelativeUrl(url) {
    return !url.startsWith("//") && !url.startsWith("http://") && !url.startsWith("https://") && !url.startsWith("file://") && !url.startsWith("data:") && !url.startsWith("blob:") ;
}

/**
 * 
 * @param {HTMLElement} element
 * @returns {Handler | undefined}
 */
function getHandler(element) {
    var tagName = element.tagName.toLowerCase();
    return handlers[tagName.toUpperCase()];
}

/**
 * 
 * @param {string} tag 
 * @param {Handler} handler 
 */
function registerHandler(tag, handler) {
    handlers[tag.toUpperCase()] = handler;
}

/**
 * @param {string} text
 * @param {string} tag
 * @param {number} level
 */
function indentText(text, tag, level) {
    return text.split("\n").map(line => tag.repeat(level) + line).join("\n");
}

/**
 * @param {string} text
 * @param {number} level
 */
function indent4Spaces(text, level = 1) {
    return indentText(text, "    ", level);
}

/**
 * 
 * @param {string} text 
 * @returns {string}
 */
function latexEscape(text) {
    return text.replace(/\\/g, "\\textbackslash{}")
        .replace(/~/g, "\\textasciitilde{}")
        .replace(/\^/g, "\\textasciicircum{}")
        .replace(/&/g, "\\&")
        .replace(/%/g, "\\%")
        .replace(/\$/g, "\\$")
        .replace(/#/g, "\\#")
        .replace(/{/g, "\\{")
        .replace(/}/g, "\\}")
        .replace(/_/g, "\\_")
        .replace(/</g, "\\textless{}")
        .replace(/>/g, "\\textgreater{}");
}

/**
 * 
 * @param {Comment} node 
 */
function toLatexComment(node) {
    return indentText(node.textContent, "% ", 1);
}

/**
 * 
 * @param {HTMLElement} element 
 * @param {ConversionContext} ctx
 * @returns {string}
 */
function convertInner(element, ctx) {
    let content = "";
    if (element.childNodes.length > 0)
        for (let child of element.childNodes)
            content += convert(child, ctx);
    if (content.length <= 0)
        content = latexEscape(element.innerText);
    return content;
}

/**
 * 
 * @param {Node} node
 * @param {ConversionContext} ctx
 * @returns {string}
 */
function convert(node, ctx) {
    if (isTextNode(node))
        return latexEscape(node.textContent);

    if (isElementNode(node)) {
        let element = /** @type {HTMLElement} */ (node);
        let handler = getHandler(element);
        if (handler)
            return handler(element, ctx);
        else {
            console.warn(`No handler for ${element.tagName}`, element);
            return latexEscape(node.innerText);
        }
    }

    if (isCommentNode(node)) {
        return toLatexComment(node) + "\n";
    }

    console.warn("Unknown node type: " + node.nodeType, node);
    return "";
}

/**
 * @param {string} tag
 * @returns { (element : HTMLElement) => string }
 */
function getHeadingHandler(tag) {

    const map = {
        "H1": "chapter",
        "H2": "section",
        "H3": "subsection",
        "H4": "subsubsection",
        "H5": "paragraph",
        "H6": "subparagraph",
    };

    return (element, ctx) => {
        return `\n\\${map[tag.toUpperCase()]}{${convertInner(element, ctx).trim()}}\n`;
    }
}

/**
 * 
 * @param {string} tag 
 */
function registerHeadingHandler(tag) {
    registerHandler(tag, getHeadingHandler(tag));
}

registerHeadingHandler("h1");
registerHeadingHandler("h2");
registerHeadingHandler("h3");
registerHeadingHandler("h4");
registerHeadingHandler("h5");
registerHeadingHandler("h6");

registerHandler("p", (element, ctx) => {
    return `\n${convertInner(element, ctx).trim()}\n`;
});

registerHandler("i-math", (element, ctx) => {
    return `$${element.tex.trim()}$`;
});

registerHandler("tex-math", (element, ctx) => {
    return `\n\\begin{equation}\n${indent4Spaces(element.tex.trim())}\n\\end{equation}\n`;
});

registerHandler("a", (element, ctx) => {
    let href = element.getAttribute("href");
    if (href) {
        href = href.trim().replaceAll("\\", "/");
        if (href.startsWith("http")) {
            addPackage(ctx, "hyperref");
            return `\\href{${href}}{${convertInner(element, ctx).trim()}}`;
        } else {
            let remoteFile = ctx.remoteFiles.find(f => f.relPath === href);
            if (remoteFile) {
                //ctx.packages.add(remoteFile.package);
                return `\\hyperref[${remoteFile.label}]{${convertInner(element, ctx).trim()}}`;
            } else {
                console.warn("Unknown link: " + href, element);
                return convertInner(element, ctx);
            }
        }
    } else {
        return convertInner(element, ctx);
    }
});

registerHandler("img", (element, ctx) => {
    let src = element.getAttribute("src");
    if (src) {
        let remoteUrl = resolveUrl(src);
        let isRelative = isRelativeUrl(src);
        let relPath = "";
        if (isRelative) {
            relPath = src;
        } else {
            throw new Error("Not implemented");
        }

        /**
         * @type {RemoteFile}
         */
        let remoteFile = {
            url: remoteUrl,
            relPath: relPath,
        }
        // TODO use a set, not an array
        ctx.remoteFiles.push(remoteFile);

        let alt = element.getAttribute("alt");
        
        addPackage(ctx, "graphicx");
        return `\\includegraphics{${relPath}}`;
    }
});

/**
 * 
 * @param {HTMLElement} element 
 * @returns {{ latex : string, packages : string[], remoteFiles: RemoteFile[] }}
 */
function convertHtmlElementContent(element) {
    /**
     * @type {ConversionContext}
     */
    let ctx = {
        remoteFiles: [],
        packages: new Set(),
    };
    
    let latex = "";
    for (let child of element.childNodes) {
        latex += convert(child, ctx);
    }

    return {
        latex,
        packages: Array.from(ctx.packages),
        remoteFiles: ctx.remoteFiles,
    }
}

let article = document.getElementsByTagName("article")[0]
if (article == undefined) {
    alert("No article found");
    //throw new Error("No article found");
} else {
    let button = document.createElement("button");
    button.innerHTML = "Convert to LaTeX";
    button.addEventListener("click", () => {
        let {
            latex,
            packages,
            remoteFiles,
        } = convertHtmlElementContent(article);

        let dirs = new Set();
        for (let remoteFile of remoteFiles) {
            let dir = parentPath(remoteFile.relPath);
            dirs.add(dir);
        }

        //alert(content);
        //document.body.innerText = content;
        document.body.innerHTML = "";
        document.body.innerHTML += `<pre><code class="language-sh">${remoteFiles.map(f => `wget "${f.url}" -o "${f.relPath}"`).join("\n")}</code></pre>`;
        document.body.innerHTML += `<pre><code class="language-latex">${packages.join("\n")}</code></pre>`;
        document.body.innerHTML += `<pre><code class="language-latex">${latex}</code></pre>`;
        hljs.highlightAll();
    });
    //article.before(button);

    // insert button before all other content in body
    document.body.insertBefore(button, document.body.firstChild);
}

