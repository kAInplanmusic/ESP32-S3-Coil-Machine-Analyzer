# 🚀 Setting Up GitHub Pages - 3 Simple Steps

## Problem
Web app showing 404 at: `https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/`

**Root Cause**: GitHub Pages is not yet enabled in repository settings.

---

## ✅ Solution: Enable GitHub Pages

### Step 1: Open Repository Settings
1. Go to: https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer
2. Click **Settings** tab (top right)
3. Wait for page to load

### Step 2: Navigate to Pages Section
1. Left sidebar → Find **"Pages"** (or scroll down)
2. Click **Pages**

### Step 3: Configure GitHub Pages
Look for "Build and deployment" section:

**Option A: Deploy from Branch** (Recommended)
```
Source: [Dropdown] → Select "Deploy from a branch"
Branch: [Dropdown] → Select "copilot/windows-linux-android-webapp"
Folder: [Dropdown] → Select "/(root)"
Click "Save"
```

**Option B: GitHub Actions** (Alternative)
```
Source: [Dropdown] → Select "GitHub Actions"
Wait 1-2 minutes, the workflow will auto-configure
```

### Step 4: Wait & Verify
1. **Wait 2-3 minutes** - GitHub Pages needs time to build
2. **Check status**: Settings → Pages → Look for green "✓ Your site is live at..."
3. **Visit URL**: https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/

---

## 🔍 Troubleshooting

### Still showing 404?
- [ ] Check that Pages setting shows "Deploy from a branch"
- [ ] Verify branch is set to "copilot/windows-linux-android-webapp"
- [ ] Wait additional 5 minutes (sometimes takes longer on first deploy)
- [ ] Clear browser cache: Ctrl+Shift+Delete
- [ ] Try incognito/private mode
- [ ] Check URL spelling carefully: `/ESP32-S3-Coil-Machine-Analyzer/`

### No "Pages" option in Settings?
- [ ] Scroll down the left sidebar (it's near the bottom)
- [ ] Or directly visit: `https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer/settings/pages`

### "Source" dropdown only shows GitHub Actions?
- [ ] Check that repository has at least one commit
- [ ] Try refreshing the page
- [ ] Use direct URL above (ends with `/settings/pages`)

---

## 📊 What Happens After You Enable It

1. **GitHub Pages builds** your web app (takes 1-2 minutes)
2. **Files deployed** from `web-app/dist/` to GitHub Pages
3. **URL goes live** at `https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/`

### App Features Available
✅ Real-time audio capture
✅ FFT frequency analysis
✅ Waveform visualization
✅ Spectrum display
✅ Measurement storage
✅ JSON/CSV export
✅ History browser

---

## 🔧 Alternative: Manual Deploy (If Settings Doesn't Work)

If the GitHub Pages UI isn't working, you can deploy manually via terminal:

```bash
# Clone and navigate to repo
cd ESP32-S3-Coil-Machine-Analyzer

# Ensure web app is built
cd web-app
npm run build

# Deploy to gh-pages branch
cd ..
git subtree push --prefix web-app/dist origin gh-pages

# Then enable GitHub Pages in Settings → Pages and select "gh-pages" branch
```

---

## 📋 Checklist

After completing the setup:

- [ ] GitHub Pages is "Enabled" (green checkmark)
- [ ] Source shows "Deploy from a branch"
- [ ] Branch is "copilot/windows-linux-android-webapp"
- [ ] Folder is "/(root)"
- [ ] Waiting 2-3 minutes...
- [ ] URL shows "Your site is live at https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/"
- [ ] Can access the web app
- [ ] Can capture audio and see results

---

## ✨ Quick Reference

**Repository**: kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer  
**Branch**: copilot/windows-linux-android-webapp  
**Build**: Web app (React + TypeScript + Vite)  
**Size**: 51 KB gzipped  
**Status**: ✅ Built and ready to deploy  

**Live URL** (once enabled):  
🌐 https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/

---

## 📞 Support

**Steps not working?**
1. Check Settings → Pages is accessible
2. Verify URL has no typos
3. Wait 5+ minutes (sometimes needs longer)
4. Try different browser
5. Clear all browser cache

**Web app won't load?**
1. Open DevTools (F12)
2. Check Console tab for errors
3. Check Network tab - all files loading?
4. Verify base URL: `/ESP32-S3-Coil-Machine-Analyzer/`

---

**That's it!** Once you enable GitHub Pages, the web app will be live. 🎉
