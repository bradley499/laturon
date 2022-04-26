var sourceFile = "user_source_file.lt";
try {
	FS.close(FS.open(sourceFile, "w+"));
} catch {
	outputMessage("Failed to generate the required files", 2);
}

async function updateSourceFile(sourceCode){
	FS.truncate(sourceFile, 0);
	FS.writeFile(sourceFile, sourceCode);
	await Module.ccall("run_file", "number", [], [], {async: true}).then(function(){}).catch(function(){
		error(0, 0);
	}).finally(function(){
		terminate();
	});
}
