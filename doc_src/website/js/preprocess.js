// @ts-check

(()=> {

class TagsPreprocessor {

    constructor() {
        /** @type {Map<string, (element: HTMLElement, preprocessor: TagsPreprocessor) => HTMLElement | HTMLElement[]>} */
        this.preprocessors = new Map();
    }

    /**
     * @param {string} tag
     * @param {(element: HTMLElement, preprocessor: TagsPreprocessor) => HTMLElement | HTMLElement[]} preprocessor
     * @returns {TagsPreprocessor}
     * */
    addPreprocessor(tag, preprocessor) {
        this.preprocessors.set(tag, preprocessor);
        return this;
    }

    /**
     * @param {HTMLElement} element
     * @returns {void}
     * */
    process(element) {
        if (!this.preprocessors.has(element.tagName))
        {
            this.processChildren(element);
            return;
        }
        let proc = this.preprocessors.get(element.tagName);
        if (!proc)
            proc = (element, preprocessor) => element;
        
        const newElement = proc(element, this);
        if (newElement instanceof HTMLElement) {
            element.replaceWith(newElement);
            this.processChildren(newElement);
        } else if (newElement instanceof Array) {
            element.replaceWith(...newElement);
            for (const child of newElement) {
                this.processChildren(child);
            }
        }
    }

    /**
     * @param {HTMLElement} element
     * @returns {void}
     * */
    processChildren(element) {
        for (const child of element.children) {
            // if child is an HtmlElement
            if (child instanceof HTMLElement)
                this.process(child);
        }
    }

    /**
     * @param {HTMLElement} element
     * @returns {boolean}
     */
    isProcessable(element) {
        return this.preprocessors.has(element.tagName);
    }
}
const preprocessor = new TagsPreprocessor();


preprocessor.addPreprocessor('RED', (element, preprocessor) => {
    const newElement = document.createElement('span');
    newElement.classList.add('red');
    newElement.innerText = element.innerText;
    preprocessor.processChildren(newElement);
    return newElement;
});

preprocessor.addPreprocessor('BLUE', (element, preprocessor) => {
    const newElement = document.createElement('span');
    newElement.classList.add('red');
    newElement.innerText = "ci";
    preprocessor.processChildren(newElement);
    return newElement;
});

// This will change every <p> tag to a <span class="p"> tag
// so that paragraphs can contain block elements
preprocessor.addPreprocessor('PARAGRAPH', (element, preprocessor) => {
    const newElement = document.createElement('span');
    newElement.classList.add('p');
    newElement.innerHTML = element.innerHTML;
    preprocessor.processChildren(newElement);
    return newElement;
});

preprocessor.processChildren(document.body);

(() => {
    const elements = document.querySelectorAll('pre[trim] > code');
    for (const code of elements) {
        if (!(code instanceof HTMLElement))
            continue;

        // remove empty lines from start and end
        const lines = code.innerHTML.split('\n');
        console.log(lines);
        let start = (() => {
            for (let i = 0; i < lines.length; i++) {
                if (lines[i].trim() !== '')
                    return i;
            }
            return lines.length;
        })();
        let end = (() => {
            for (let i = lines.length - 1; i >= 0; i--) {
                if (lines[i].trim() !== '')
                    return i;
            }
            return -1;
        })();
        if (start > end) {
            code.innerHTML = '';
            continue;
        }

        // remove common indent
        const minIndent = Math.min(...lines.slice(start, end + 1).map(line => line.search(/\S/)));

        code.innerHTML = lines.slice(start, end + 1).map(line => line.substring(minIndent)).join('\n');
        const pre = /** @type{HTMLElement} */(code.parentElement);
        pre.setAttribute("data-code", code.innerText);
    }
})();

(() => {
    const elements = document.querySelectorAll('pre[numbering] > code');
    for (const code of elements) {
        if (!(code instanceof HTMLElement))
            continue;
        const pre = /** @type{HTMLElement} */(code.parentElement);
        const startLine = parseInt(pre.getAttribute('start-line') || '1');

        const lines = code.innerHTML.split('\n');
        const newLines = lines.map((line, index) => `<span class="line-number">${index + startLine}</span>${line}`);
        code.innerHTML = newLines.join('\n');
    }
})();

(() => {
    const elements = document.querySelectorAll('pre[copy-button]');
    for (const pre of elements) {
        if (!(pre instanceof HTMLElement))
            continue;
        const button = document.createElement('button');
        button.classList.add('copy-button');
        button.innerText = 'Copy';
        button.addEventListener('click', () => {
            const code = pre.getAttribute("data-code") || pre.innerText;
            console.log(code);
            navigator.clipboard.writeText(code);
        });
        pre.appendChild(button);
    }
})();


})();