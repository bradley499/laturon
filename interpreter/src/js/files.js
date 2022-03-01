var sourceFile = "user_source_file";
try {
	FS.close(FS.open(sourceFile, "w+"));
} catch {
	outputMessage("Failed to generate the required files", 2);
}

function updateSourceFile(sourceCode){
	FS.truncate(sourceFile, 0);
	FS.writeFile(sourceFile, sourceCode);
}