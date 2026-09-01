#!/usr/bin/env python3
# Decodes ASAM MDF files produced by the CAN MDF Logger

# Imports ----------------------------------------------------------------------------------

from __future__ import annotations

import re
import sys
import argparse

from pathlib import Path
from typing import Iterable

# Globals ---------------------------------------------------------------------------------

SESSION_PATTERN = re.compile (r"^session_(\d+)$")
SPLIT_PATTERN = re.compile (r"^split_(\d+)\.mf4$", re.IGNORECASE)

# Functions --------------------------------------------------------------------------------

def find (path: Path) -> list[tuple[str, list[Path]]]:
	if path.is_file ():
		return [(str (path), [path])] if path.suffix.lower () == ".mf4" else []

	if path.is_dir ():
		session_directories = []
		path_match = SESSION_PATTERN.match (path.name)
		directories = [path] if path_match else path.iterdir ()
		for directory in directories:
			match = SESSION_PATTERN.match (directory.name)
			if directory.is_dir () and match:
				split_files = []
				for file_path in directory.iterdir ():
					split_match = SPLIT_PATTERN.match (file_path.name)
					if file_path.is_file () and split_match:
						split_files.append ((int (split_match.group (1)), file_path))

				if split_files:
					session_directories.append (
						(int (match.group (1)), str (directory), [file for _, file in sorted (split_files)])
					)

		return [
			(session, files)
			for _, session, files in sorted (session_directories)
		]

	return []


def decode (sessions: Iterable[tuple[str, list[Path]]], can1_dbc: Path, can2_dbc: Path) -> list[Path]:
	try:
		from asammdf import MDF

	except ImportError as error:
		raise RuntimeError(
			"MDF decoding requires asammdf; install it with 'pip install asammdf'"
		) from error

	outputs = []
	for session, split_files in sessions:
		try:
			mdf = MDF.concatenate ([str (file_path) for file_path in split_files])
			decoded = mdf.extract_bus_logging (
				database_files = {
					"CAN": [
						(str (can1_dbc), 1),
						(str (can2_dbc), 2),
					]
				}
			)
			output_directory = Path (session)
			if output_directory.is_file ():
				output_directory = output_directory.parent
			output = output_directory / "decoded.mf4"
			decoded.save (str (output), overwrite = True)
			for split_file in split_files:
				split_file.unlink ()
			outputs.append (output)

		except Exception as error:
			raise RuntimeError (f"failed to concatenate, decode, or save session '{session}': {error}") from error

	return outputs


def init_parser () -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser (
		description = "Decode ASAM MDF CAN data using CAN 1 and CAN 2 DBC files."
	)
	parser.add_argument (
		"path",
		type = Path,
		help = "MDF file or session directory",
	)
	parser.add_argument (
		"can1_dbc",
		type = Path,
		help = "DBC file for CAN 1",
	)
	parser.add_argument (
		"can2_dbc",
		type = Path,
		help = "DBC file for CAN 2",
	)
	return parser


def main () -> int:
	args = init_parser ().parse_args ()
	for argument_name, argument_path in (("MDF path", args.path), ("CAN 1 DBC", args.can1_dbc), ("CAN 2 DBC", args.can2_dbc)):
		if not argument_path.exists ():
			print (f"error: {argument_name} does not exist: {argument_path}", file = sys.stderr)
			return 2

	sessions = find (args.path)
	if not sessions:
		print (
			f"error: no MDF files (.dat, .mdf, or .mf4) found in '{args.path}'",
			file = sys.stderr,
		)
		return 2

	try:
		outputs = decode (sessions, args.can1_dbc, args.can2_dbc)
	except RuntimeError as error:
		print (f"error: {error}", file = sys.stderr)
		return 1

	for output in outputs:
		print (f"decoded MDF written to '{output}'")

	return 0

if __name__ == "__main__":
	raise SystemExit (main ())
