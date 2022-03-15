"use strict";

(async () => {
	const chromium = (() => {
		try {
			if (typeof navigator.userAgentData !== "undefined") {
				for (let i = 0; i < navigator.userAgentData.brands.length; i++) {
					if (["Chromium", "Google Chrome"].indexOf(navigator.userAgentData.brands[i].brand) >= 0) {
						if (navigator.userAgentData.brands[i].version > 85) {
							return true;
						}
					}
				}
				return false;
			}
			if (navigator.userAgent.match(/Chrome\/\d+/) !== null) {
				const raw = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
				return raw ? (parseInt(raw[2], 10) > 85) : false;
			}
		} catch { }
		return false;
	})();
	let worker = false;
	let executing = false;
	let interpreterReady = false;
	const interactsContainer = document.createElement("div");
	const loading = document.createElement("div");
	const interacts = [document.createElement({ true: "div", false: "textarea" }[chromium]), document.createElement("div")];
	const buttons = [document.createElement("div"), document.createElement("div"), document.createElement("div"), document.createElement("div")];
	loading.id = "loading";
	const authorPicture = await fetch("https://api.github.com/users/bradley499").then(response => response.json()).then(async data => {
		return await fetch(data["avatar_url"]).then(image => image.blob()).catch(error => {
			throw new Error(error);
		});
	}).catch(error => {
		return null;
	});
	const authorPictureBlob = (window.URL || window.webkitURL).createObjectURL(authorPicture);
	let authorPictureImg = document.createElement("img");
	authorPictureImg.src = authorPictureBlob;
	let authorPictureDirection = (() => {
		if (typeof gazeDetection !== "undefined") {
			return gazeDetection.default;
		} else {
			return 0;
		}
	})();
	(async () => {
		if (typeof gazeDetection === "undefined") {
			return;
		}
		authorPictureDirection = await new Promise((resolve, reject) => {
			let attempts = 0;
			let loop;
			const isReady = () => {
				if (gazeDetection.ready) {
					clearInterval(loop);
					resolve(true);
				}
				if (attempts < 50) {
					attempts++;
				} else {
					clearInterval(loop);
					reject(false);
				}
			};
			loop = setInterval(isReady, 200);
		}).then(async v => {
			return await gazeDetection.direction(authorPictureImg);
		}).catch(v => {
			return gazeDetection.default;
		});
	})();
	const alertBuilder = async (title, message, buttons, input) => {
		if (chromium) {
			interacts[0].removeAttribute("contenteditable");
		} else {
			interacts[0].setAttribute("disabled", "disabled");
		}
		const overlay = document.createElement("div");
		let response = await new Promise((resolve, reject) => {
			overlay.id = "alertOverlay";
			const container = document.createElement("div");
			const containerContents = [document.createElement("div"), document.createElement("div"), document.createElement("div")];
			containerContents[0].id = "authorAlertPicture";
			containerContents[0].style.backgroundImage = "url(\"" + authorPictureBlob + "\")";
			if (authorPictureDirection === 1) {
				containerContents[0].classList.add("authorAlertPictureLeft");
			}
			containerContents[2].id = "alertContents";
			let containerContentsDynamic = [];
			if (title != null && title.trim().length > 0) {
				let titleHeading = document.createElement("h2");
				titleHeading.innerText = title.trim();
				containerContentsDynamic.push(titleHeading);
			}
			if (message != null && message.trim().length > 0) {
				message = message.trim();
				message = message.split("\n");
				message.forEach(paragraph => {
					let paragraphElement = document.createElement("p");
					paragraphElement.innerText = paragraph;
					containerContentsDynamic.push(paragraphElement);
				});
			}
			let inputElement = null;
			if (input != null && input.trim().length > 0) {
				input = input.trim("\n");
				inputElement = document.createElement("input");
				inputElement.placeholder = input;
				inputElement.classList.add("alertContentsInput");
				containerContentsDynamic.push(inputElement);
				buttons = ["Confirm", "Cancel"];
			}
			let containerContentsDynamicButtons = [];
			if (buttons == null || buttons.length == 0) {
				buttons = ["Close"];
			} else if (typeof buttons == "string") {
				buttons = [buttons];
			}
			let buttonNumber = 0;
			buttons.forEach(button => {
				let buttonElement = document.createElement("div");
				buttonElement.innerText = button;
				buttonElement.setAttribute("rel", buttonNumber);
				buttonElement.addEventListener("click", (e) => {
					let number = parseInt(e.target.getAttribute("rel"));
					if (isNaN(number)) {
						number = 0;
					}
					if (inputElement != null) {
						if (number == 0) {
							resolve(inputElement.value.trim());
						} else {
							resolve(null);
						}
					}
					resolve(number);
				});
				buttonNumber++;
				containerContentsDynamicButtons.push(buttonElement);
			});
			const containerContentsDynamicScrollable = [document.createElement("div"), document.createElement("div")];
			containerContentsDynamicScrollable[0].id = "alertContentsMain";
			containerContentsDynamicScrollable[1].id = "alertButtons";
			containerContentsDynamic.forEach(element => {
				containerContentsDynamicScrollable[0].appendChild(element);
			});
			containerContentsDynamicButtons.forEach(element => {
				containerContentsDynamicScrollable[1].appendChild(element);
			});
			containerContents[1].appendChild(containerContentsDynamicScrollable[0]);
			containerContents[1].appendChild(containerContentsDynamicScrollable[1]);
			containerContents[2].appendChild(containerContents[1]);
			container.appendChild(containerContents[0]);
			container.appendChild(containerContents[2]);
			container.id = "alertContainer";
			overlay.appendChild(container);
			document.body.appendChild(overlay);
			containerContentsDynamicScrollable[0].focus();
		}).then(number => {
			overlay.remove();
			return number;
		});
		if (chromium) {
			interacts[0].setAttribute("contenteditable", "true");
		} else {
			interacts[0].removeAttribute("disabled");
		}
		return response;
	}
	const loadingState = (message, active) => {
		if (!active) {
			loading.style.display = "none";
			interacts[1].classList.remove("loading");
			return;
		}
		loading.style.display = "inline-block";
		loading.innerText = message;
		interacts[1].classList.add("loading");
	}
	loadingState(null, false);
	const changeExecutionState = (button) => {
		if (!interpreterReady) {
			alertBuilder("Unable to run", "Sorry, but the interpreter is still being loaded... Until the interpreter has loaded, you'll have to wait before you can run your program.", null, null);
			return;
		}
		let state = buttonData[2]["state"];
		tooltipIter(buttons[2], 2);
		if (state == 0 && button != null) {
			inputOutput[1].innerHTML = "";
			buttons[2].classList.add("executing");
			interactsContainer.classList.add("executing");
			executing = true;
			if (buttonData[3]["state"] == 3) {
				resizeEditor();
			}
			worker.postMessage({ "type": "sourceCode", "data": interacts[0].innerText });
			loadingState("Tokenising source code", true);
			return;
		}
		buttons[2].classList.remove("executing");
		interactsContainer.classList.remove("executing");
		executing = false;
		loadingState(null, false);
	};
	const load = async (button) => {
		let empty = false;
		if (chromium) {
			empty = (interacts[0].innerText.trim().length == 0);
		} else {
			empty = (interacts[0].value.trim().length == 0);
		}
		if (!empty) {
			if (await alertBuilder("You have unsaved work", "Are you sure you want to load a new file, whilst you're still working on something?\nAll progress of your current project will be lost.", ["Load a file", "Cancel"], null) == 1) {
				return;
			}
		}
		let element = document.createElement("input");
		element.type = "file";
		element.addEventListener("change", async () => {
			if (element.files.length == 0) {
				return;
			}
			let file = element.files[0];
			if (file.type.slice(0, 4) != "text") {
				return await alertBuilder("Unable to read that file", "Sorry, but that sort of file is not able to be read.", null, null);
			}
			let reader = new FileReader();
			reader.addEventListener("load", function (event) {
				let text = event.target.result.replace(/\t/g, "    ");
				if (chromium) {
					interacts[0].innerHTML = "";
				} else {
					interacts[0].value = "";
				}
				updateEditor(null);
				if (chromium) {
					interacts[0].focus();
					document.execCommand("insertText", false, text);
				} else {
					interacts[0].value = text;
					interacts[0].focus();
				}
			});
			reader.readAsText(file);
		});
		element.click();
	};
	const save = async (button) => {
		tooltipIter(button.target, 1);
		let empty = false;
		if (chromium) {
			empty = (interacts[0].innerText.trim().length == 0);
		} else {
			empty = (interacts[0].value.trim().length == 0);
		}
		if (empty) {
			return alertBuilder("Unable to download", "Sorry, but you're unable to download an empty file.", "Continue coding", null);
		}
		let element = document.createElement("a");
		if (chromium) {
			element.setAttribute("href", "data:text/plain;charset=utf-8," + encodeURIComponent(interacts[0].innerText));
		} else {
			element.setAttribute("href", "data:text/plain;charset=utf-8," + encodeURIComponent(interacts[0].value));
		}
		let name = " ";
		while (name.trim().length == 0) {
			name = await alertBuilder("Download file", { true: "Your file name cannot be blank.\n", false: "" }[name == ""] + "What would you like to name your file?", null, "Download name");
			if (name == null) {
				return;
			}
		}
		element.setAttribute("download", name + ".code");
		element.click();
		element.remove();
	};
	const resizeEditor = () => {
		const sizes = ["editorSizing5050", "editorSizing7030", "editorSizing3070", "editorSizing100"];
		sizes.forEach(size => {
			interactsContainer.classList.remove(size)
		});
		switch (buttonData[3]["state"]) {
			case 0:
				interactsContainer.classList.add(sizes[1]);
				break;
			case 1:
				interactsContainer.classList.add(sizes[2]);
				break;
			case 2:
				interactsContainer.classList.add(sizes[{ true: 0, false: 3 }[executing]]);
				if (executing) {
					buttonData[3]["state"] = 3;
				}
				break;
			case 3:
				interactsContainer.classList.add(sizes[0]);
				break;
		}
		buttonData[3]["state"]++;
		if (buttonData[3]["state"] == 4) {
			buttonData[3]["state"] = 0;
		}
	};
	const tooltipIter = (button, rel) => {
		let iter = parseInt(button.getAttribute("state"));
		if (iter >= (buttonData[rel]["tooltip"].length - 1)) {
			iter = 0;
			button.title = buttonData[rel]["tooltip"][iter];
		} else {
			button.title = buttonData[rel]["tooltip"][++iter];
		}
		buttonData[rel]["state"] = iter;
	};
	const updateEditor = (e) => {
		if (!chromium) {
			return;
		}
		if (interacts[0].innerHTML.trim().length == 0) {
			if (e != null && e.keyCode == 8) {
				e.preventDefault();
			}
			let line = document.createElement("span");
			line.className = "line";
			line.innerText = " ";
			interacts[0].appendChild(line);
		}
	};
	const buttonContainer = document.createElement("div");
	buttonContainer.id = "buttonContainer";
	const buttonData = [{ "id": "load", "tooltip": ["Load source code from file"], "function": load, "state": 0 }, { "id": "save", "tooltip": ["Download source code", "Downloading source code"], "function": save, "state": 0 }, { "id": "start_stop", "tooltip": ["Run program", "Stop program"], "function": changeExecutionState, "state": 0 }, { "id": "resize", "tooltip": ["Resize editor"], "function": resizeEditor, "state": 0 }]
	for (let i = 0; i < buttons.length; i++) {
		buttons[i].id = buttonData[i]["id"];
		buttons[i].title = buttonData[i]["tooltip"][0];
		buttons[i].classList = "operationButton";
		if (i != 3) {
			buttons[i].setAttribute("state", buttonData[i]["state"]);
		}
		buttons[i].addEventListener("click", buttonData[i]["function"]);
		buttonContainer.appendChild(buttons[i]);
	}
	interactsContainer.id = "interactsContainer";
	interactsContainer.classList.add("editorSizing5050");
	interacts[0].id = "editor";
	interacts[0].classList = "interactive";
	interacts[0].setAttribute("contenteditable", "true");
	if (chromium) {
		interacts[0].addEventListener("keypress", updateEditor);
		interacts[0].addEventListener("keyup", updateEditor);
		interacts[0].addEventListener("keydown", (e) => {
			if (e.keyCode == 9) {
				e.preventDefault();
				document.execCommand("insertText", false, "    ");
			}
		});
	}
	interacts[0].addEventListener("paste", (e) => {
		if (!chromium) {
			return;
		}
		e.preventDefault();
		let text = e.clipboardData.getData("text/plain");
		let temp = document.createElement("div");
		temp.value = text.replace(/(<([^>]+)>)/ig, "").replace(/\t/g, "    ");
		document.execCommand("insertText", false, temp.value);
		temp.remove();
		const selection = window.getSelection();
		if (!selection.rangeCount) {
			return;
		}
		const firstRange = selection.getRangeAt(0);
		if (firstRange.commonAncestorContainer === document) {
			return;
		}
		const tempAnchorEl = document.createElement("br");
		firstRange.insertNode(tempAnchorEl);
		tempAnchorEl.scrollIntoView({
			block: "end",
		});
		tempAnchorEl.remove();
		interacts[0].focus();
	});
	interacts[1].id = "runner";
	interacts[1].classList = "interactive";
	interacts[1].appendChild(loading);
	const inputOutput = [[document.createElement("div"), document.createElement("input"), document.createElement("input")], document.createElement("div")];
	const inputEnabler = (enabled) => {
		for (let i = 1; i < 3; i++) {
			if (enabled) {
				inputOutput[0][i].removeAttribute("disabled");
			} else {
				inputOutput[0][i].setAttribute("disabled", "disabled");
			}
		}
		if (enabled) {
			inputOutput[0][2].title = " Submit input";
			inputOutput[0][1].placeholder = " Please type your input here...";
			inputOutput[0][0].removeAttribute("title");
			inputOutput[0][0].classList.remove("disabled");
		} else {
			inputOutput[0][1].placeholder = "Input is currently disabled!";
			inputOutput[0][0].title = "Input is currently disabled!";
			inputOutput[0][0].classList.add("disabled");
		}
	};
	const newOutput = (type, message) => {
		let output = document.createElement("p");
		output.classList.add("output");
		if (type != null) {
			output.classList.add(type);
			output.classList.add("systemic")
		}
		output.innerText = message;
		inputEnabler(false);
		inputOutput[1].appendChild(output);
		inputOutput[1].scrollTop = inputOutput[1].scrollHeight;
		if (type == "error") {
			changeExecutionState(null);
		}
	};
	inputOutput[0][2].type = "button";
	inputOutput[0][0].id = "input";
	inputEnabler(false);
	inputOutput[0][0].appendChild(inputOutput[0][1]);
	inputOutput[0][0].appendChild(inputOutput[0][2]);
	inputOutput[1].id = "output";
	interacts[1].append(inputOutput[1]);
	interacts[1].append(inputOutput[0][0]);
	interactsContainer.appendChild(interacts[0]);
	interactsContainer.appendChild(interacts[1]);
	// Interface ready for rendering
	document.body.classList.remove("loading");
	document.body.innerHTML = "";
	document.body.appendChild(buttonContainer);
	document.body.appendChild(interactsContainer);
	loadingState("Loading...", true);
	updateEditor();
	// Web worker data transition
	(async () => {
		const interpreterData = await fetch("./assets/js/interpreter.json").then(response => response.json()).then(response => {
			return response;
		});
		worker = new Worker("./assets/js/" + interpreterData["worker"]);
		worker.onmessage = (e) => {
			e = e.data;
			switch (e["type"]) {
				case "loading":
					loadingState(["Loading interpreter...", null, "Parsing source code", null][e["state"]], (e["state"] != 1 && e["state"] != 3));
					if (e["state"] == 1 && !interpreterReady) {
						interpreterReady = true;
					}
					break;
				case "output":
					newOutput(["info", "warning", "error", "generic"][e["state"]], e["message"]);
					break;
			}
		};
	})();
})();