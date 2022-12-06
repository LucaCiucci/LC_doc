// @ts-check

var index_elementss = []

var id_counter = 0

/**
 * creates an incremental id
 * @returns {string}
 * @param {string} prefix
 */
function make_id() {
	return "tmp_ID_" + (id_counter++).toString()
}

/**
 * 
 * @param {HTMLElement} element 
 * @returns {string}
 */
function element_id(element) {
	if (element.id.length <= 0) {
		element.id = make_id()
	}
	return element.id;
}

/**
 * @type { Set<string> }
 */
var used_headings_ids = new Set();

/**
 * 
 * @param {HTMLElement} element 
 * @returns {string}
 */
 function heading_id(element) {
	if (element.id.length <= 0) {
		// get the text of the heading
		let text = element.innerText;
		// remove all non alphanumeric characters but keep spaces
		text = text.replace(/[^a-zA-Z0-9 ]/g, "");
		// replace spaces with dashes, if multiple spaces are present, replace them with a single dash
		text = text.replace(/ +/g, "-");
		// make it lowercase
		text = text.toLowerCase();
		
		let counter = 0;
		let id = text;
		while (used_headings_ids.has(id)) {
			counter++;
			id = text + "-" + counter.toString();
		}
		used_headings_ids.add(id);
		element.id = id;

		// place all the content of the heading in a link
		let link = document.createElement("a");
		link.setAttribute("href", "#" + id);
		// move all the children of the heading to the link
		while (element.firstChild) {
			link.appendChild(element.firstChild);
		}
		// add the link to the heading
		element.appendChild(link);
	}
	return element.id;
}

function make_index() {

	/** @type {HTMLElement} */

	/** @var {HTMLElement | Null} article ciao */
	let article = document.getElementsByTagName("article")[0]
	
	let indexes = document.getElementsByTagName("lc-nav-index");

	index_elementss = [];

	for (let index of indexes) {
		//let index = indexes[i];

		index.innerHTML = ""
		let index_a = document.createElement("aside")
		index.appendChild(index_a)
		index_a.innerHTML += "<h3>In this article</h3>"
		let index_elements = []

		//let cc = article.querySelectorAll("h2, h3")
		// see https://stackoverflow.com/questions/3680876/using-queryselectorall-to-retrieve-direct-children
		let cc = article.querySelectorAll(":scope > h2, :scope >  h3")
		for (let c of cc) {
			e = document.createElement("li")
			a = document.createElement("a")
			e.appendChild(a)
			a.innerHTML = c.innerHTML
			a.setAttribute("href", "#" + heading_id(c))
			e.data_element = c
			c.data_index_element = e
			index_a.appendChild(e)
			index_elements.push(e)
			//e.addEventListener("click", function() {
			//    window.scrollTo(c.getBoundingClientRect().x, c.getBoundingClientRect().y);
			//})

			if (c.tagName == "H3") {
				e.className = "sub"
			}

		}

		index_elementss.push(index_elements)
	}
}

function onscroll() {
	/**
	 * 
	 * @param {Element} elm
	 * @param {number} offset
	 * @returns {boolean}
	 */
	function checkVisible(elm) {
		// https://stackoverflow.com/questions/5353934/check-if-element-is-visible-on-screen
		var rect = elm.getBoundingClientRect();
		var viewHeight = Math.max(document.documentElement.clientHeight, window.innerHeight);
		return !(rect.bottom < 0 || rect.top - viewHeight >= 0);
	}

	function checkAbove(elm) {
		var rect = elm.getBoundingClientRect();
		return rect.bottom < 0;
	}

	function checkAboveOrSlightlyAbove(elm) {
		var rect = elm.getBoundingClientRect();
		return rect.top <= 1;
	}

	for (let index_elements of index_elementss) {
		for (let is in index_elements) {
			let i = parseInt(is);
			let e = index_elements[i];

			/**
			 * @param {HTMLElement} elm
			 * @param { "active" | "slightly-active" | "none" } attr
			 */
			function setAttr(elm, attr) {
				if (attr != "none") {
					elm.setAttribute(attr, true);
				}

				if (attr != "active") {
					elm.removeAttribute("active");
				}

				if (attr != "slightly-active") {
					elm.removeAttribute("slightly-active");
				}
			}

			if (checkVisible(e.data_element)) {
				setAttr(e, "active");
				continue;
			}

			// if last element
			if (i == index_elements.length - 1) {
				if (checkAbove(e.data_element)) {
					setAttr(e, "active");
				} else {
					setAttr(e, "none");
				}
				continue;
			}

			if (checkAbove(e.data_element) && !checkAboveOrSlightlyAbove(index_elements[i + 1].data_element)) {
				setAttr(e, "slightly-active");
				continue;
			}

			setAttr(e, "none");
		}
	}

	/*for (let e of index_elements) {

		if (checkAbove(e.data_element)) {
			e.setAttribute("active", "true")
		} else {
			e.removeAttribute("active")
		}
	}*/

	return;

	lastKnownScrollPosition = window.scrollY;

	for (let e of index_elements) {
		e.removeAttribute("active");
	}

	for (let i = 0; i < index_elements.length; i++) {
		e = index_elements[i].data_element
		if (i == 0) {
			if (lastKnownScrollPosition <= e.getBoundingClientRect().y) {
				e.data_index_element.setAttribute("active", "")
				/*
				https://developer.mozilla.org/en-US/docs/Web/API/Element/scrollIntoView
				https://stackoverflow.com/questions/56688002/javascript-scrollintoview-only-in-immediate-parent
				ma non funziona...
				*/
				//e.scrollIntoView({ behavior: 'smooth', block: 'nearest', inline: 'start' })
			}
		}
		if (i == index_elements.length - 1) {
			if (lastKnownScrollPosition >= e.getBoundingClientRect().y) {
				e.data_index_element.setAttribute("active", "")
				//e.scrollIntoView({ behavior: 'smooth', block: 'nearest', inline: 'start' })
			}
		} else {
			e2 = index_elements[i + 1].data_element
			if (lastKnownScrollPosition >= e.getBoundingClientRect().y && lastKnownScrollPosition < e2.getBoundingClientRect().y) {
				e.data_index_element.setAttribute("active", "")
				//e.scrollIntoView({ behavior: 'smooth', block: 'nearest', inline: 'start' })
			}
		}
	}
}

// ================================================================

var lc_fig_counter = 0;

/**
 * A lc-figure component.
 * 
 * slots:
 *  - graphics
 *  - caption
 * 
 * if no ID is specified, a unique ID is generated
 */
class LCFigure extends HTMLElement {
	constructor() {
		super();

		this.fig_number = ++lc_fig_counter;

		if (this.id.length <= 0) {
			this.id = "fig_" + this.fig_number;
		}

		this.attachShadow({mode: 'open'});

		this.shadowRoot.innerHTML += "<style>a { color: var(--link-color)}</style>"

		this.content = document.createElement("div");
		let content = this.content;
		this.shadowRoot.appendChild(content);
		content.style.display = "flex"
		content.style.gap = "0.5em"
		content.style.flexDirection = "column";
		content.style.alignContent = "center";

		let graphics = document.createElement("slot");
		content.appendChild(graphics);
		graphics.name = "graphics";

		let description = document.createElement("div");
		content.appendChild(description);
		description.style.fontSize = "75%";

		description.innerHTML = 'Fig. <a href="#' + this.id + '">' + this.fig_number + "</a>: ";
		let caption = document.createElement("slot");
		description.appendChild(caption);
		caption.name = "caption";
		//label.style.fontStyle = "italic";
		description.style.maxHeight = "10em";
		description.style.overflow = "auto";
	}
}
customElements.define('lc-figure', LCFigure);

// ================================================================

make_index()
onscroll()
document.addEventListener("scroll", onscroll)

// ================================================================

const key = '.';
if (false) {
	let link = document.getElementById("edit-link");
	if (link) {
		let text = document.createTextNode(`press "${key}" to edit this file`);
		link.parentNode.append(text);
	}

}
document.addEventListener('keypress', function(e) {
	if (e.key == key) {
		e.preventDefault();
		e.stopPropagation();
		
		let link = document.getElementById("edit-link");
		if (link) {
			link.click();
		}
	}
});

function show_view_source() {
	window.alert("to view the raw html as presented by the browser, right click on the page and select 'view source'");
}

let view_source_link = document.getElementById("view-source-link");
if (view_source_link) {
	view_source_link.href = `javascript:show_view_source();`;
}


function escape_html(str) {
	return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#039;');
}

/**
 * 
 * @param {string} id 
 * @param {string} url 
 * @param {string} language
 */
function fetch_code(id, url, language = "") {
	fetch(url)
		.then(response => response.text())
		.then(text => {
			let code = document.getElementById(id);
			code.innerHTML = escape_html(text);
			code.className = language ? "language-" + language : "no-highlight";
			hljs.highlightElement(code);
		});
}

// all elements with "lc-code-src" attribute
document.querySelectorAll("[lc-code-src]").forEach(e => {
	let url = e.getAttribute("lc-code-src");
	let _class = e.getAttribute("class");
	console.log(e);
	fetch(url)
		.then(async response => {
			let code = e;
			if (response.ok) {
				return await response.text();
			} else {
				code.className = "no-highlight";
				code.innerHTML = '<b style="color:red">error: ' + response.status + " " + response.statusText + "\nurl: " + url + "</b>";
				//code.innerHTML = "ERROR: " + response.status + " " + response.statusText;
				//throw new Error("HTTP error " + response.status);
				return undefined;
			}
		})
		.then(text => {
			if (!text)
				return;
			let code = e;
			console.log(code);
			code.innerHTML = escape_html(text);
			code.className = _class;
			hljs.highlightElement(code);
		});
});

function lc_use() {
	let lc_uses = document.querySelectorAll("lc-use");
	lc_uses.forEach(e => {
		if (e.hasAttribute("ref")) {
			let ref = e.getAttribute("ref");
			if (ref) {
				if (ref.startsWith("#")) ref = ref.substring(1);
				let ref_element = document.getElementById(ref);
				if (ref_element) {
					let fragment = document.createDocumentFragment();
					for (let i = 0; i < ref_element.children.length; i++) {
						fragment.appendChild(ref_element.children[i].cloneNode(true));
					}
					e.parentNode?.replaceChild(fragment, e);
				}
			}
		}
	});
}
lc_use();