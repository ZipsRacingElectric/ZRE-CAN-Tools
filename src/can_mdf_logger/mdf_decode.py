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

# Match session folders (i.e. session_##)
# These are expected to contain split MDF fragments for a single logging session.
SESSION_PATTERN = re.compile (r"^session_(\d+)$")

# Match individual MDF fragments (i.e. split_##.mf4)
# The numeric portion is used to sort the fragments in the correct time order.
SPLIT_PATTERN = re.compile (r"^split_(\d+)\.mf4$", re.IGNORECASE)

# Functions --------------------------------------------------------------------------------

# Locate valid MDF sessions & collect their split files in order.
# Returns a list of (session_path, [split_file_1, split_file_2, ...]) tuples.
def find (path: Path) -> list[tuple[str, list[Path]]]:

	# If the given path is a single file, only accept an MF4 file.
	if path.is_file ():
		return [(str (path), [path])] if path.suffix.lower () == ".mf4" else []

	# If the given path is a directory, scan it for session folders.
	if path.is_dir ():
		session_directories = []
		path_match = SESSION_PATTERN.match (path.name)

		# If the input itself is already a session directory, only inspect it.
		# Otherwise, scan the parent directory for session_## folders.
		directories = [path] if path_match else path.iterdir ()
		for directory in directories:
			match = SESSION_PATTERN.match (directory.name)
			if directory.is_dir () and match:
				split_files = []
				for file_path in directory.iterdir ():
					split_match = SPLIT_PATTERN.match (file_path.name)
					if file_path.is_file () and split_match:
						# Store the file alongside its numeric split index so they can be sorted.
						split_files.append ((int (split_match.group (1)), file_path))

				# Only keep sessions that actually contain split MDF files.
				if split_files:
					session_directories.append (
						(int (match.group (1)), str (directory), [file for _, file in sorted (split_files)])
					)

		# Return sorted session entries by session number.
		return [
			(session, files)
			for _, session, files in sorted (session_directories)
		]

	return []


# Decode one or more MDF sessions using DBC definitions for CAN bus 1 & CAN bus 2.
# Each session is reconstructed from its split files, decoded to a merged MF4 log,
# and the original split fragments are removed after a successful save.
def decode (sessions: Iterable[tuple[str, list[Path]]], can1_dbc: Path, can2_dbc: Path) -> list[Path]:
	try:
		# Import lazily so the dependency is only required when this code path is used.
		from asammdf import MDF

	except ImportError as error:
		raise RuntimeError(
			"MDF decoding requires asammdf; install it with 'pip install asammdf'"
		) from error

	outputs = []
	for session, split_files in sessions:
		try:
			# Merge all split MDF files into a single in-memory MDF object.
			mdf = MDF.concatenate ([str (file_path) for file_path in split_files])

			# Decode CAN traffic using the two DBC files.
			# Each DBC is assigned to a CAN bus number, matching the logger's bus mapping.
			decoded = mdf.extract_bus_logging (
				database_files = {
					"CAN": [
						(str (can1_dbc), 1),
						(str (can2_dbc), 2),
					]
				}
			)

			# Save the decoded file alongside the session directory.
			output_directory = Path (session)
			if output_directory.is_file ():
				output_directory = output_directory.parent
			output = output_directory / "decoded.mf4"
			decoded.save (str (output), overwrite = True)

			# Once the merged output has been written, clean up the temporary split files.
			for split_file in split_files:
				split_file.unlink ()
			outputs.append (output)

		except Exception as error:
			raise RuntimeError (f"failed to concatenate, decode, or save session '{session}': {error}") from error

	return outputs


# Build the command-line arguments for the utility.
def init_parser () -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser (
		description = "Decode ASAM MDF CAN data using CAN 1 & CAN 2 DBC files."
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

# Main Gateway ------------------------------------------------------------------------------

# Validates paths, finds sessions, decodes each one, & reports the created files.
def main () -> int:
	args = init_parser ().parse_args ()

	# Validate the existence of all user-provided input files/paths.
	for argument_name, argument_path in (("MDF path", args.path), ("CAN 1 DBC", args.can1_dbc), ("CAN 2 DBC", args.can2_dbc)):
		if not argument_path.exists ():
			print (f"error: {argument_name} does not exist: {argument_path}", file = sys.stderr)
			return 2

	# Discover MDF sessions from the supplied path.
	sessions = find (args.path)
	if not sessions:
		print (
			f"error: no MDF files (.dat, .mdf, or .mf4) found in '{args.path}'",
			file = sys.stderr,
		)
		return 2

	# Decode all discovered sessions & report any failure.
	try:
		outputs = decode (sessions, args.can1_dbc, args.can2_dbc)
	except RuntimeError as error:
		print (f"error: {error}", file = sys.stderr)
		return 1

	# Print the generated output files for each session.
	for output in outputs:
		print (f"decoded MDF written to '{output}'")

	return 0

if __name__ == "__main__":
	raise SystemExit (main ())
