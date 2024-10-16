// @ts-check

/** @type {{[key in SourceTypes]: string}} */
const types = {
	"gh": "Guitar Hero",
	"rb": "Rock Band",
	"game": "Games",
	"charter": "Charter",
	"custom": "Custom"
}

class SourceElement extends HTMLElement {
	/** @param {Source} source */
	constructor(source) {
		super();

		const template = document.getElementById("source_template");
        if(!(template instanceof HTMLTemplateElement)) throw new Error("Source template not existent on DOM");

        this.replaceChildren(template.content.cloneNode(true));

		this.assignIcon(source.icon);
		this.assignName(source.names["en-US"]);
		this.assignTags(source.ids);
	}

	/**
	 * @param {string} iconName 
	 */
	assignIcon(iconName) {
		const iconContainer = this.querySelector(".icon");
		if(!iconContainer) return;

		const primary = `../../base/icons/${iconName}.png`;
		const secondary = `../../extra/icons/${iconName}.png`;

		const imageElement = iconContainer.querySelector("img");
		if(imageElement) {
			imageElement.src = primary;

			imageElement.onerror = () => {
				imageElement.src = secondary;
				imageElement.onerror = null;
			}
		}
	}

	/**
	 * @param {string} sourceName 
	 */
	assignName(sourceName) {
		const nameContainer = this.querySelector(".name");
		if(!nameContainer) return;

		nameContainer.textContent = sourceName;

		const imageContainer = this.querySelector("img");
		if(imageContainer) {
			imageContainer.alt = `${sourceName} icon`;
		}
	}

	/**
	 * @param {string[]} tags 
	 */
	assignTags(tags) {
		const tagsContainer = this.querySelector(".tags");
		if(!tagsContainer) return;

		const tagsElements = tags.map(tagName => {
			const tagElement = document.createElement("div");
			tagElement.classList.add("tag");

			tagElement.textContent = tagName;
			return tagElement;
		});

		tagsContainer.append(...tagsElements);
	}

}

if ('customElements' in window) {
	customElements.define('open-source', SourceElement);
}

/**
 * Creates a type section
 * @param {string} id 
 * @param {string} name 
 */
function createTypeElement(id, name) {
	const typesContainer = document.getElementById("types");

	const container = getTemplate("type_template");

	if(container) {
		container.id = id;
		const nameContainer = container.querySelector(".name");
		nameContainer?.replaceChildren(new Text(name));

		typesContainer?.append(container);
	}
}

/**
 * Creates a new stat for a type.
 * @param {string} id 
 * @param {string} name 
 */
function createStatElement(id, name) {
	const statsContainer = document.getElementById("stats");

	const container = getTemplate("stat_template");

	if(container) {
		container.id = `${id}-stat`;

		const nameContainer = container.querySelector(".name");
		nameContainer?.replaceChildren(new Text(name));

		const linkElement = container.querySelector("a");
		if(linkElement) {
			linkElement.href = `#${id}`;
		}

		statsContainer?.append(container);
	}
}

/**
 * Creates a clone from template
 * @param {string} templateId 
 * @returns {HTMLElement|undefined}
 */
function getTemplate(templateId) {
	const template = document.getElementById(templateId);
	if(!(template instanceof HTMLTemplateElement)) throw new Error(`${templateId} not existent on DOM`);
	
	const container = template.content.firstElementChild?.cloneNode(true);

	if(container instanceof HTMLElement) {
		return container;
	}

	return;
}

function processTypes() {
	Object.keys(types).forEach(type => {
		const typeName = types[type];

		createTypeElement(type, typeName);
		createStatElement(type, typeName);
	});
}

async function loadSources() {
	const base = await fetch("../../base/index.json").then(res => res.json());
	const extra = await fetch("../../extra/index.json").then(res => res.json());

	[base, extra].forEach(sourceList => {
		sourceList.sources.forEach(
			/** @param {Source} source  */
			source => {
				const sourceElement = new SourceElement(source);
				const typeContainer = document.querySelector(`#${source.type} > .sources`);
				typeContainer?.append(sourceElement);

				refreshStatCount(source.type);
			}
		);
	});
}

/**
 * Counts all sources in a type section and changes the stat count
 * @param {SourceTypes} typeId 
 */
function refreshStatCount(typeId) {
	const sourcesContainer = document.querySelector(`#${typeId} > .sources`);
	const counterContainer = document.querySelector(`#${typeId}-stat .counter`);
	if(!sourcesContainer || !counterContainer) throw new Error();

	const count = sourcesContainer.childElementCount;
	counterContainer.replaceChildren(new Text(count.toString()));
}

processTypes();
loadSources();

/**
 * Type definitions
 */

/**
 * @typedef {Object} Source
 * @property {string[]} ids - An array of IDs.
 * @property {{"en-US": string, [key: string]: string }} names - Object containing the source name in different languages.
 * @property {string} icon - The icon for the source.
 * @property {SourceTypes} type - The type of the source.
 */

/**
 * @typedef {"custom"|"game"|"charter"|"rb"|"gh"} SourceTypes
 */