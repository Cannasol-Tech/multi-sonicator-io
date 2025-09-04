import os
import json
from datetime import datetime

def parse_gcov_coverage(file_path):
    lines_covered = 0
    lines_total = 0
    with open(file_path) as f:
        for line in f:
            if ':' in line:
                count, code = line.split(':', 1)
                count = count.strip()
                if count != '-' and count != '#####':
                    lines_total += 1
                    if int(count) > 0:
                        lines_covered += 1
    return lines_covered, lines_total

def main():
    owner = "Cannasol-Tech"
    repo = "multi-sonicator-io"
    releaseTag = os.getenv("RELEASE_TAG", "v0.0.0")
    commit = os.getenv("COMMIT_SHA", "unknown")
    createdAt = datetime.utcnow().isoformat() + "Z"
    modules = ["communication", "control", "hal", "sonicator"]
    totals = {"lines": {"covered": 0, "total": 0}}
    files = []
    for module in modules:
        cov_path = f"test/unit/{module}/coverage.txt"
        if os.path.exists(cov_path):
            covered, total = parse_gcov_coverage(cov_path)
            totals["lines"]["covered"] += covered
            totals["lines"]["total"] += total
            files.append({"path": cov_path, "lines": {"covered": covered, "total": total}})
    pct = (totals["lines"]["covered"] / totals["lines"]["total"] * 100) if totals["lines"]["total"] else 0
    totals["lines"]["pct"] = round(pct, 2)
    summary = {
        "version": "1.0.0",
        "owner": owner,
        "repo": repo,
        "releaseTag": releaseTag,
        "commit": commit,
        "createdAt": createdAt,
        "totals": totals,
        "files": files
    }
    os.makedirs("final", exist_ok=True)
    with open("final/coverage-summary.json", "w") as f:
        json.dump(summary, f, indent=2)
    print("Coverage summary written to final/coverage-summary.json")

if __name__ == "__main__":
    main()
