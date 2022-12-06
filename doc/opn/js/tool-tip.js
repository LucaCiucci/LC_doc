// typescript checks
// @ts-check

var tooltipCfg = {
	showDelay: 1000,
	hideDelay: 500,
}

class ToolTipElement extends HTMLElement {
	constructor() {
		super();

		/** @type { number | null } */
		this.timeout = null;
		this.showing = false;

		this.addEventListener('mouseenter', () => {
			this.setTimeout(() => this.showTooltip(), tooltipCfg.showDelay);
		});

		this.addEventListener('mouseleave', () => {
			this.setTimeout(() => this.hideTooltip(), tooltipCfg.hideDelay);
		});
	}

	removeTimeout() {
		if (!this.timeout) return;

		clearTimeout(this.timeout);
		this.timeout = null;
	}

	/**
	 * @param { () => void } f
	 * @param { number } delay
	 */
	setTimeout(f, delay) {
		this.removeTimeout();
		this.timeout = setTimeout(f, delay);
	}

	/**
	 * @returns { HTMLElement | undefined }
	 */
	popup() {
		let tt = /** @type { HTMLElement | undefined }*/(this.getElementsByTagName('tooltip-popup')[0]);

		if (!tt && this.kind() !== 'content') {
			tt = document.createElement('tooltip-popup');
			this.appendChild(tt);
		}

		return tt;
	}

	wait(delay = 0) {
		return new Promise(resolve => setTimeout(resolve, delay));
	}

	showTooltip() {
		if (this.showing) return;
		let popup = this.popup();
		if (!popup) return;

		this.populatePopup(popup);
		this.setPopupPosition(popup);
		popup.setAttribute('pre_visible', '');

		// wait for the tooltip to become block
		// https://stackoverflow.com/questions/21539276/how-to-correctly-wait-until-javascript-applies-inline-css
		this.wait().then(() => popup?.setAttribute('visible', ''));

		this.showing = true;
	}

	hideTooltip = () => {
		if (!this.showing) return;
		let tt = this.popup();
		if (!tt) return;
		tt.removeAttribute('visible');
		tt.removeAttribute('pre_visible');
		this.showing = false;
	}

	/**
	 * 
	 * @param { HTMLElement } popup 
	 */
	setPopupPosition(popup) {
		const trect = this.getBoundingClientRect();
		const top = trect.top;// this.offsetTop;
		const left = trect.left;// this.offsetLeft;
		const to = top + this.offsetHeight + 5;
		const lo = left + this.offsetWidth / 2;

		popup.style.top = to + 'px';
		popup.style.left = lo + 'px';

		const rect = popup.getBoundingClientRect();

		if (rect.right > window.innerWidth) {
			popup.style.left = (lo - (rect.right - window.innerWidth) - 5) + 'px';
		}

		if (rect.left < 0) {
			popup.style.left = (lo - rect.left + 5) + 'px';
		}
	}

	/**
	 * @returns { "content" | "use" | "href" | "iframe" }
	 */
	kind() {
		if (this.hasAttribute('use'))
			return "use";
		if (this.hasAttribute('href'))
			return "href";
		if (this.hasAttribute('iframe'))
			return "iframe";
		return "content";
	}

	/**
	 * 
	 * @param { HTMLElement } popup 
	 */
	populatePopup(popup) {
		let ty = this.kind();

		switch (ty) {
			case "content":
				//popup.innerHTML = popup.innerHTML;
				break;
			case "use":
			{
				let use = this.getAttribute('use');
				if (!use) return;
				if (use.startsWith('#')) use = use.substr(1);
				let el = document.getElementById(use);
				if (!el) return;
				popup.innerHTML = el.innerHTML;
			}
				break;
			case "href":
			{
				let href = this.getAttribute('href');
				if (!href) return;
				popup.innerHTML = `<a href="${href}">${href}</a>`;
			}
				break;
			case "iframe":
			{
				let href = this.getAttribute('iframe');
				if (!href) return;
				popup.innerHTML = `<iframe src="${href}"></iframe>`;
			}
				break;
			default:
				throw new Error("Unknown tooltip type: " + ty);
				break;
		}
	}
}

customElements.define('tool-tip', ToolTipElement);