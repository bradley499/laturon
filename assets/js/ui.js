"use strict";

(async () => {
	const chromium = (() => {
		try {
			if (typeof navigator.userAgentData !== "undefined") {
				for (let i = 0; i < navigator.userAgentData.brands.length; i++) {
					if (["Chromium", "Google Chrome"].indexOf(navigator.userAgentData.brands[i].brand) >= 0) {
						if (navigator.userAgentData.brands[i].version > 85) {
							if (navigator.userAgentData.mobile) {
								return false;
							}
							return true;
						}
					}
				}
				return false;
			}
			if (/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|Android|Silk|lge |maemo|midp|mmp|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows (ce|phone)|xda|xiino/i.test(navigator.userAgent) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(navigator.userAgent.substr(0, 4))) {
				return false;
			}
			if (navigator.userAgent.match(/Chrome\/\d+/) !== null) {
				const raw = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
				return raw ? (parseInt(raw[2], 10) > 85) : false;
			}
		} catch { }
		return false;
	})();
	let interpreterData = null;
	let initialExecution = true;
	let executed = false;
	let worker = false;
	let executing = false;
	let interpreterReady = false;
	const interactsContainer = document.createElement("div");
	const loading = document.createElement("div");
	const interacts = [document.createElement({ true: "div", false: "textarea" }[chromium]), document.createElement("div")];
	interacts[0].setAttribute("spellcheck", false);
	interacts[0].setAttribute("autocorrect", "off");
	interacts[0].setAttribute("autocapitalize", "none");
	interacts[0].setAttribute("autocomplete", "off");
	const buttons = [document.createElement("div"), document.createElement("div"), document.createElement("div"), document.createElement("div"), document.createElement("div")];
	loading.id = "loading";
	let authorPictureBlob = null;
	const authorPicture = await fetch("https://api.github.com/users/bradley499").then(response => response.json()).then(async data => {
		return await fetch(data["avatar_url"]).then(image => image.blob()).catch(error => {
			return null;
		});
	}).catch(error => {
		return null;
	});
	let authorPictureImg = document.createElement("img");
	if (authorPicture == null) {
		authorPictureBlob = null;
	} else {
		authorPictureBlob = (window.URL || window.webkitURL).createObjectURL(authorPicture);
		authorPictureImg.src = authorPictureBlob;
	}
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
	window.onresize = () => {
		if (buttonData[2]["state"] == 3) {
			if (window.innerWidth > 600) {
				changeExecutionState(null);
			}
		}
	};
	const changeExecutionState = async (button) => {
		if (!interpreterReady) {
			if (buttonData[2]["state"] == 3) {
				buttons[2].classList.remove("back");
				buttons[2].classList.remove("executing");
				interactsContainer.classList.remove("executing");
				buttonData[2]["state"] = 0;
				executing = false;
				loadingState(null, false);
				return;
			}
			if (initialExecution) {
				alertBuilder("Unable to run", "Sorry, but the interpreter is still being loaded... Until the interpreter has loaded, you'll have to wait before you can run your program.", null, null);
			} else {
				buttons[3].style.display = "inline-block";
				buttons[2].style.display = "none";
				await new Promise((resolve) => {
					setInterval(() => {
						if (interpreterReady) {
							resolve();
						}
					}, 300);
				});
				buttons[3].style.display = "none";
				buttons[2].style.display = "inline-block";
			}
			return;
		}
		let state = buttonData[2]["state"];
		if (state == 0 && button != null) {
			inputEnabler(false);
			tooltipIter(buttons[2], 2);
			inputOutput[1].innerHTML = "";
			buttons[2].classList.add("executing");
			executing = true;
			executed = true;
			if (buttonData[3]["state"] == 3) {
				resizeEditor();
			}
			let sourceCode = "";
			if (chromium) {
				sourceCode = (interacts[0].innerText || "").split("\n\n").join("\n");
			} else {
				sourceCode = interacts[0].value;
			}
			buttons[2].setAttribute("state", 1);
			worker.postMessage({ "type": "sourceCode", "data": sourceCode });
			interactsContainer.classList.add("executing");
			loadingState("Tokenising source code", true);
			return;
		} else if (buttonData[2]["state"] != 3) {
			worker.terminate();
			inputOutput[0][1].placeholder = "Program has finished execution!";
			inputOutput[0][0].title = "Program has finished execution!";
			if (buttonData[2]["state"] != 2 && buttonData[2]["state"] != 4) {
				newOutput("error", "Execution of program was terminated.", false);
			}
			newWorker(interpreterData, true);
		}
		if (buttonData[2]["state"] == 3) {
			buttons[2].classList.remove("back");
			buttons[2].classList.remove("executing");
			interactsContainer.classList.remove("executing");
			tooltipIter(buttons[2], 2);
			buttons[2].setAttribute("state", 0);
			buttonData[2]["state"] = 0;
		} else if (window.innerWidth > 600) {
			buttons[2].classList.remove("executing");
			interactsContainer.classList.remove("executing");
			tooltipIter(buttons[2], 2);
			buttons[2].setAttribute("state", 0);
			buttonData[2]["state"] = 0;
		} else {
			buttons[2].classList.add("back");
			buttons[2].title = "Back to editor";
			buttonData[2]["state"] = 3;
		}
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
		element.accept = ".lt";
		element.addEventListener("change", async () => {
			if (element.files.length == 0) {
				return;
			}
			let file = element.files[0];
			if (!file.name.endsWith(".lt")) {
				if (await alertBuilder("Unrecognised file type", "Sorry, but that sort of file is not recognised, would you still like to load it?", ["Yes", "No"], null) == 1) {
					return;
				}
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
		element.setAttribute("download", name + ".lt");
		element.click();
		element.remove();
	};
	const resizeEditor = () => {
		if (interpreterData === false) {
			interactsContainer.classList.add("editorSizing100");
			return;
		}
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
	const buttonData = [{ "id": "load", "tooltip": ["Load source code from file"], "function": load, "state": 0 }, { "id": "save", "tooltip": ["Download source code", "Downloading source code"], "function": save, "state": 0 }, { "id": "start_stop", "tooltip": ["Run program", "Stop program"], "function": changeExecutionState, "state": 0 }, { "id": "loadingSpinner", "tooltip": ["Loading..."], "function": () => { }, "state": 0 }, { "id": "resize", "tooltip": ["Resize editor"], "function": resizeEditor, "state": 0 }]
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
	const spinner = document.createElement("div");
	spinner.id = "loadingSpinner";
	buttons[3].appendChild(spinner);
	buttons[3].style.display = "none";
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
	let inputState = false;
	const inputEnabler = (enabled) => {
		for (let i = 1; i < 3; i++) {
			if (enabled) {
				inputOutput[0][i].removeAttribute("disabled");
			} else {
				inputOutput[0][i].setAttribute("disabled", "disabled");
			}
		}
		inputState = enabled;
		if (enabled) {
			inputOutput[0][2].title = " Submit input";
			inputOutput[0][1].placeholder = " Please type your input here...";
			inputOutput[0][0].removeAttribute("title");
			inputOutput[0][0].classList.remove("disabled");
			inputOutput[0][1].focus();
		} else {
			inputOutput[0][1].value = "";
			if (executed) {
				inputOutput[0][1].placeholder = "Input is currently disabled!";
				inputOutput[0][0].title = "Input is currently disabled!";
			}
			inputOutput[0][0].classList.add("disabled");
		}
	};
	const inputSender = (e) => {
		e.preventDefault();
		if (!inputState) {
			return;
		}
		worker.postMessage({ "type": "input", "state": 1, "message": inputOutput[0][1].value });
		inputEnabler(false);
	};
	const newOutput = (type, message, errorOut = true) => {
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
	};
	inputOutput[0][2].type = "button";
	inputOutput[0][0].id = "input";
	inputEnabler(false);
	inputOutput[0][1].placeholder = "";
	inputOutput[0][0].title = "";
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
	const newWorker = (interpreterData, silentCreation = false) => {
		if (interpreterData === false) {
			alertBuilder("Failed to load", "Sorry, but unfortunately the information required to load up the interpreter failed to load.\nThis means that you will be unable to execute your program, however you are still able to: load, edit, and save; your source code.", null, null);
			return;
		}
		interpreterReady = false;
		worker = new Worker("./assets/js/" + interpreterData["worker"]);
		worker.postMessage({ "type": "startup", "data": [initialExecution, silentCreation] });
		worker.onmessage = (e) => {
			e = e.data;
			switch (e["type"]) {
				case "loading":
					if (!(e["state"] == 0 && !initialExecution)) {
						loadingState(["Loading interpreter...", null, "Parsing source code", null, null][e["state"]], (e["state"] != 1 && e["state"] < 3));
						initialExecution = false;
					}
					if (e["state"] == 1 && !interpreterReady) {
						interpreterReady = true;
					}
					break;
				case "input":
					inputEnabler(true);
					inputOutput[0][2].addEventListener("click", inputSender);
					break;
				case "output":
					newOutput(["info", "warning", "error", "generic"][e["state"]], e["message"]);
					if (e["state"] != 2) {
						break;
					}
				case "terminate":
					inputEnabler(false);
					buttonData[2]["state"] = 2;
					inputOutput[0][1].placeholder = "Program has finished execution!";
					inputOutput[0][0].title = "Program has finished execution!";
					changeExecutionState(null);
					break;
			}
		};
	};
	(async () => {
		interpreterData = await fetch("./assets/js/interpreter.json").then(response => response.json()).then(response => {
			return response;
		}).catch(r => {
			return false;
		});
		if (interpreterData === false) {
			buttons[2].remove();
			buttons[3].remove();
			buttons[4].remove();
			resizeEditor();
		}
		const supportsWebAssembly = (() => {
			try {
				if (typeof WebAssembly === "object"
					&& typeof WebAssembly.instantiate === "function") {
					const module = new WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
					if (module instanceof WebAssembly.Module)
						return new WebAssembly.Instance(module) instanceof WebAssembly.Instance;
				}
			} catch (e) {}
			return false;
		})();
		if (!window.Worker || !supportsWebAssembly || (typeof Int8Array === "undefined" || typeof Int16Array === "undefined" || typeof Int32Array === "undefined" || typeof Uint8Array === "undefined" || typeof Uint16Array === "undefined" || typeof Uint32Array === "undefined" || typeof Float32Array === "undefined" || typeof Float64Array === "undefined" || typeof BigInt64Array === "undefined" || typeof BigUint64Array === "undefined")) {
			alertBuilder("Browser not supported", "Sorry, but your browser is not supported due to it have missing (or disabled) features that are required to run the Laturon IDE.", null, null);
			buttons[2].remove();
			buttons[3].remove();
			buttons[4].remove();
			resizeEditor();
		} else {
			newWorker(interpreterData);
		}
	})();
})();