# 🔄 How to Move Commits from Wrong Repository to Correct Repository

If commits were accidentally made to the wrong repository, there are several ways to move them. The method depends on the number of commits and complexity of the situation.

---

## 📋 Diagnostics: Which Commits Need to Be Moved?

First, identify which commits belong to the target repository:

```bash
# View files in a specific commit
git show --name-only <commit-hash>

# View commits affecting a specific folder
git log --oneline -- path/to/folder/

# Export list of commits with files for analysis
git log --format="%H %s" --name-only > commits_list.txt
```

---

## 🔧 Methods for Moving Commits

### ✅ Method 1: `git cherry-pick` (for individual commits)

Suitable when you need to move **specific individual commits**.

```bash
# 1. In the correct repository, create a branch for import
cd /path/to/correct-repo
git checkout -b import-from-wrong-repo

# 2. Add the wrong repository as remote
git remote add wrong-repo /path/to/wrong-repo
# or by URL:
# git remote add wrong-repo https://github.com/user/wrong-repo.git

# 3. Fetch commits (without merging)
git fetch wrong-repo

# 4. Cherry-pick needed commits
git cherry-pick <commit-hash>
# or a range:
git cherry-pick <start-hash>^..<end-hash>

# 5. If conflicts occur - resolve and continue:
git cherry-pick --continue
# or skip the problematic commit:
git cherry-pick --skip

# 6. After successful transfer, remove temporary remote
git remote remove wrong-repo
```

> ⚠️ **Important**: `cherry-pick` creates **new commits** with different hashes. Original commits in wrong-repo remain unchanged.

---

### ✅ Method 2: `git format-patch` + `git am` (for commit series)

Suitable when you need to move **a sequence of commits while preserving history**.

```bash
# In the wrong repository:
cd /path/to/wrong-repo

# Create patches for needed commits
git format-patch <start-commit>^..<end-commit> -o /tmp/patches

# In the correct repository:
cd /path/to/correct-repo
git checkout -b import-branch

# Apply patches
git am /tmp/patches/*.patch

# If conflicts occur:
git am --resolve    # after manual fixes
git am --continue   # or
git am --abort      # to cancel
```

> ✅ Advantages: preserves author, date, and commit message.

---

### ✅ Method 3: `git filter-repo` (filtering by file paths)

🔥 **Recommended tool** for complex filtering. Replaces deprecated `git filter-branch`.

```bash
# Installation (if not available):
pip install git-filter-repo
# or: https://github.com/newren/git-filter-repo

# 1. Clone wrong repository to temporary folder
git clone /path/to/wrong-repo /tmp/filtered-repo
cd /tmp/filtered-repo

# 2. Keep only needed paths (everything else will be removed from history)
git filter-repo --path path/to/project/ --path another/file.txt --force

# 3. Add correct repository as remote
git remote add correct /path/to/correct-repo
git fetch correct

# 4. Transfer filtered branch
git push correct HEAD:refs/heads/imported-branch
```

> 🎯 Can filter by:
> - `--path <pattern>` - keep only specified paths
> - `--invert-paths` - exclude specified paths
> - `--path-rename old/:new/` - rename paths during transfer

---

### ✅ Method 4: Manual Export/Import via Archive

Suitable when **history is not important**, only changes are needed.

```bash
# In wrong-repo: export changes between commits
git diff <old-commit> <new-commit> > changes.patch

# In correct-repo: apply as one commit
git apply changes.patch
git add .
git commit -m "Import changes from wrong-repo: <description>"
```

---

## 🧹 Cleanup of Wrong Repository (Optional)

If you want to **remove transferred commits** from wrong-repo:

```bash
# 1. Create backup branch just in case
git branch backup-before-cleanup

# 2. Reset branch to state before wrong commits
git reset --hard <last-good-commit>

# 3. If commits were already pushed - force push required (careful!):
git push origin <branch> --force-with-lease

# 4. Or use git rebase to rewrite history:
git rebase -i <last-good-commit>
# In editor, delete lines with unwanted commits (drop)
```

> ⚠️ **Warning**: `--force` may affect other developers. Coordinate beforehand!

---

## 📊 Method Comparison

| Method | Preserves History | Filters by Files | Complexity | When to Use |
|--------|-------------------|------------------|------------|-------------|
| `cherry-pick` | ❌ New hashes | ❌ Manual | ⭐ Low | 1-10 individual commits |
| `format-patch` + `am` | ✅ Yes | ❌ Manual | ⭐⭐ Medium | Series of sequential commits |
| `git filter-repo` | ✅ Yes (rewritten) | ✅ Automatic | ⭐⭐⭐ High | Complex filtering by paths |
| Manual diff/apply | ❌ One commit | ✅ Yes | ⭐ Low | Code only, history not important |

---

## 🛡️ Best Practices

1. **Make backup before any operations**:
   ```bash
   git clone /path/to/repo /path/to/repo-backup
   ```

2. **Work in separate branch**, not in `main/master`:
   ```bash
   git checkout -b import-work
   ```

3. **Check result before push**:
   ```bash
   git log --oneline --graph --all
   git diff origin/main...HEAD
   ```

4. **Use `--dry-run` where possible**:
   ```bash
   git filter-repo --path src/ --dry-run
   ```

5. **Document the transfer** - create a commit with description:
   ```bash
   git commit --allow-empty -m "
   Import: transfer commits from wrong-repo
   
   Transferred commits:
   - abc123: feature X
   - def456: fix Y
   
   Source: https://github.com/user/wrong-repo
   "
   ```

---

## 🚀 Quick Checklist

```bash
# 1. Identify needed commits
git log --oneline --name-only | grep -A5 "important-file"

# 2. In correct-repo create branch and add wrong-repo
git checkout -b import-branch
git remote add wrong /path/to/wrong-repo
git fetch wrong

# 3. Transfer (choose one method):
git cherry-pick abc123 def456
# or
git format-patch ... && git am ...
# or
git filter-repo --path src/ --force && git push correct HEAD:imported

# 4. Check and remove temporary remote
git log --oneline
git remote remove wrong

# 5. (Optional) Clean up wrong-repo
# See "Cleanup of Wrong Repository" section above
```

---

## 🔍 Additional Considerations

### Handling Large Repositories
For repositories with extensive history, consider:
```bash
# Shallow clone to speed up operations
git clone --depth 50 /path/to/wrong-repo /tmp/shallow-repo

# Use --force for filter-repo on large repos
git filter-repo --path src/ --force --replace-refs delete-no-add
```

### Dealing with Submodules
If commits involve submodule changes:
```bash
# Update submodules after transfer
git submodule update --init --recursive

# Check submodule status
git submodule status
```

### Merge Conflicts Resolution
Advanced conflict resolution strategies:
```bash
# Use our version for all conflicts
git checkout --ours -- <file>
git add <file>
git cherry-pick --continue

# Use their version for all conflicts
git checkout --theirs -- <file>
git add <file>
git cherry-pick --continue
```

### Verification After Transfer
Always verify the transfer was successful:
```bash
# Compare file contents between repositories
diff -r /path/to/wrong-repo/path/to/files /path/to/correct-repo/path/to/files

# Check that no files were missed
git ls-tree -r --name-only HEAD | sort > current_files.txt
git ls-tree -r --name-only <original-commit> | sort > original_files.txt
diff current_files.txt original_files.txt
```

---

## 📞 Troubleshooting

### Common Issues and Solutions

**Issue**: `git cherry-pick` fails with merge conflicts
```bash
# Solution: Abort and try different approach
git cherry-pick --abort
# Use format-patch method instead
```

**Issue**: `git filter-repo` runs out of memory
```bash
# Solution: Increase memory or split operation
git filter-repo --path src/ --force --replace-refs delete-no-add
# Or process paths separately
git filter-repo --path src/module1/ --force
git filter-repo --path src/module2/ --force
```

**Issue**: Patches don't apply cleanly with `git am`
```bash
# Solution: Use 3-way merge
git am --3way /tmp/patches/*.patch
# Or apply manually and commit
git apply /tmp/patches/*.patch --reject
# Resolve .rej files manually, then:
git add .
git commit -m "Manual import of changes"
```

### Recovery from Mistakes
If something goes wrong during transfer:
```bash
# Always have a backup branch
git checkout backup-before-import

# Or reset to original state
git reset --hard HEAD~5  # Go back 5 commits
git reflog               # Find lost commits
git reset --hard <commit-hash>  # Restore specific commit
```
