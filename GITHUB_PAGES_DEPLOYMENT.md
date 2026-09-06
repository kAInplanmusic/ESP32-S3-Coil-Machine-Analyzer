# 🚀 GitHub Pages Deployment Guide

## Issue
The web app is showing 404 on GitHub Pages because the deployment hasn't been properly configured yet.

## ✅ What's Working
- ✅ Web app builds successfully
- ✅ App runs locally at `http://localhost:4173/ESP32-S3-Coil-Machine-Analyzer/`
- ✅ All features implemented (audio, FFT, visualization, storage)
- ✅ Build artifacts ready in `web-app/dist/`

## 🔧 How to Deploy to GitHub Pages

### Option 1: Manual Deploy (Immediate)

**Step 1**: Navigate to your GitHub repository settings
- Go to: https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer
- Click **Settings** tab

**Step 2**: Enable GitHub Pages
- Left sidebar → **Pages**
- Under "Build and deployment":
  - Source: Select **Deploy from a branch**
  - Branch: Select **gh-pages** (if exists) or **main**
  - Folder: Select **root** or **/docs** (depending on setup)
  - Click **Save**

**Step 3**: Create gh-pages branch with built files
```bash
cd web-app/dist
# Package these files for deployment

# Or use git subtree:
git subtree push --prefix web-app/dist origin gh-pages
```

**Step 4**: Verify
- Wait 1-2 minutes
- Visit: https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/
- Should see: Coil Machine Analyzer web app

### Option 2: Automated Deploy (GitHub Actions)

The workflow is already configured at `.github/workflows/web-deploy.yml`

**To activate**:
1. Update branch trigger in workflow to **main** ✅ (already done)
2. Ensure GitHub Pages is enabled in repo settings (see above)
3. Workflow will auto-deploy on next push

### Option 3: Using Deploy to GitHub Pages Tool

If you have access to GitHub Copilot or want to use our pre-configured workflow:

```bash
# Push the latest code
git push origin copilot/windows-linux-android-webapp

# Manually trigger GitHub Actions workflow
# Go to: Actions tab → Web Deploy → Run workflow
```

---

## 📋 Troubleshooting

### "Still getting 404"
- ✓ Check Settings → Pages → ensure it's enabled
- ✓ Wait 2-3 minutes for initial deployment
- ✓ Clear browser cache (Ctrl+Shift+Del)
- ✓ Try incognito/private mode
- ✓ Verify URL: `https://username.github.io/ESP32-S3-Coil-Machine-Analyzer/`

### "Deploy failed"
- ✓ Check Actions tab for workflow errors
- ✓ Ensure `web-app/dist/index.html` exists and is not empty
- ✓ Verify branch/folder settings are correct
- ✓ Try re-triggering workflow (Actions → Web Deploy → Run)

### "App loads but shows blank page"
- ✓ Open browser DevTools (F12)
- ✓ Check Console for errors
- ✓ Verify all assets load correctly
- ✓ Check that base URL is correct: `/ESP32-S3-Coil-Machine-Analyzer/`

---

## 🔍 Current Status

**Repository**: kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer
**Branch**: copilot/windows-linux-android-webapp (or main)
**Web App Build**: ✅ 51 KB gzip (all files present)
**Desktop App**: ✅ 116 MB AppImage built
**Workflow**: ✅ Configured and ready
**GitHub Pages**: ⏳ Needs activation in repo settings

---

## 📱 Local Testing (While Waiting for GitHub Pages)

Test the app locally before GitHub Pages goes live:

```bash
cd web-app
npm install
npm run dev
# Opens at http://localhost:5173/ESP32-S3-Coil-Machine-Analyzer/
```

Or use preview build:
```bash
cd web-app
npm run preview
# Opens at http://localhost:4173/ESP32-S3-Coil-Machine-Analyzer/
```

---

## ✨ Once Deployed

After GitHub Pages is live, the app will be available at:

**🌐 https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/**

**Features available**:
- ✅ Real-time audio capture
- ✅ FFT frequency analysis
- ✅ Live waveform visualization
- ✅ Frequency spectrum view
- ✅ Measurement storage
- ✅ JSON/CSV export
- ✅ History browser
- ✅ Quality scoring

**Usage**:
1. Click "Start Measurement"
2. Make sound near microphone (3-10 seconds)
3. Click "Stop Measurement"
4. View results, download as JSON/CSV
5. Browse history of all measurements

---

## 🎯 Next Steps

1. **Enable GitHub Pages** in repository settings (Settings → Pages)
2. **Select branch**: Choose `gh-pages` or run workflow to create it
3. **Wait 2-3 minutes** for initial deployment
4. **Visit URL**: https://kAInplanmusic.github.io/ESP32-S3-Coil-Machine-Analyzer/
5. **Start analyzing**! 🎉

---

**Questions?** Check the Actions tab for detailed build logs, or re-run the workflow with "Run workflow" button.
