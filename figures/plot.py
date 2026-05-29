import csv, os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ---- Style ----
plt.rcParams.update({
    'font.family':       'serif',
    'font.size':         11,
    'axes.titlesize':    12,
    'axes.labelsize':    11,
    'xtick.labelsize':   10,
    'ytick.labelsize':   10,
    'legend.fontsize':   10,
    'figure.dpi':        150,
    'axes.spines.top':   False,
    'axes.spines.right': False,
    'axes.grid':         True,
    'grid.alpha':        0.3,
    'grid.linestyle':    '--',
})

BTREE_COLOR = '#2c6fad'
LSM_COLOR   = '#c0392b'

def load(path):
    ns, bt, lsm = [], [], []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            ns.append(int(row['n']))
            bt.append(float(row['btree_ms']))
            lsm.append(float(row['lsm_ms']))
    return ns, bt, lsm

fig_dir = os.path.join(os.path.dirname(__file__))

# ================================================================
# Figure 1 – Insert throughput
# ================================================================
ns, bt, lsm = load(os.path.join(fig_dir, 'data_insert.csv'))

fig, ax = plt.subplots(figsize=(5.5, 3.8))
ax.plot(ns, bt,  '-o', color=BTREE_COLOR, label='B-Tree (T=64)', linewidth=1.8, markersize=5)
ax.plot(ns, lsm, '-s', color=LSM_COLOR,   label='LSM-Tree',      linewidth=1.8, markersize=5)
ax.set_xlabel('Number of insertions (n)')
ax.set_ylabel('Wall-clock time (ms)')
ax.set_title('Insert performance vs. dataset size')
ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{int(x):,}'))
ax.legend()
fig.tight_layout()
fig.savefig(os.path.join(fig_dir, 'fig_insert.pdf'), bbox_inches='tight')
fig.savefig(os.path.join(fig_dir, 'fig_insert.png'), bbox_inches='tight')
plt.close()
print("fig_insert saved")

# ================================================================
# Figure 2 – Lookup latency
# ================================================================
ns, bt, lsm = load(os.path.join(fig_dir, 'data_lookup.csv'))

fig, ax = plt.subplots(figsize=(5.5, 3.8))
ax.plot(ns, bt,  '-o', color=BTREE_COLOR, label='B-Tree (T=64)', linewidth=1.8, markersize=5)
ax.plot(ns, lsm, '-s', color=LSM_COLOR,   label='LSM-Tree',      linewidth=1.8, markersize=5)
ax.set_xlabel('Dataset size (n keys)')
ax.set_ylabel('Total lookup time (ms, 1,000 queries)')
ax.set_title('Point lookup performance vs. dataset size')
ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{int(x):,}'))
ax.legend()
fig.tight_layout()
fig.savefig(os.path.join(fig_dir, 'fig_lookup.pdf'), bbox_inches='tight')
fig.savefig(os.path.join(fig_dir, 'fig_lookup.png'), bbox_inches='tight')
plt.close()
print("fig_lookup saved")

# ================================================================
# Figure 3 – Range query
# ================================================================
ns, bt, lsm = load(os.path.join(fig_dir, 'data_range.csv'))

fig, ax = plt.subplots(figsize=(5.5, 3.8))
ax.plot(ns, bt,  '-o', color=BTREE_COLOR, label='B-Tree (T=64)', linewidth=1.8, markersize=5)
ax.plot(ns, lsm, '-s', color=LSM_COLOR,   label='LSM-Tree',      linewidth=1.8, markersize=5)
ax.set_xlabel('Dataset size (n keys)')
ax.set_ylabel('Total range query time (ms, 1,000 queries)')
ax.set_title('Range query performance vs. dataset size')
ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{int(x):,}'))
ax.legend()
fig.tight_layout()
fig.savefig(os.path.join(fig_dir, 'fig_range.pdf'), bbox_inches='tight')
fig.savefig(os.path.join(fig_dir, 'fig_range.png'), bbox_inches='tight')
plt.close()
print("fig_range saved")

# ================================================================
# Figure 4 – Summary bar chart (n=500k)
# ================================================================
categories = ['Insert', 'Point Lookup', 'Range Query']
bt_vals  = [141.4, 109.9, 63.7]
lsm_vals = [236.5, 1186.1, 808.4]

x = np.arange(len(categories))
width = 0.35

fig, ax = plt.subplots(figsize=(5.5, 3.8))
bars1 = ax.bar(x - width/2, bt_vals,  width, label='B-Tree',   color=BTREE_COLOR, alpha=0.88)
bars2 = ax.bar(x + width/2, lsm_vals, width, label='LSM-Tree', color=LSM_COLOR,   alpha=0.88)

ax.set_ylabel('Time (ms)')
ax.set_title('Operation comparison at n = 500,000')
ax.set_xticks(x)
ax.set_xticklabels(categories)
ax.legend()

# value labels on top of bars
for bar in bars1:
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 8,
            f'{bar.get_height():.0f}', ha='center', va='bottom', fontsize=9)
for bar in bars2:
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 8,
            f'{bar.get_height():.0f}', ha='center', va='bottom', fontsize=9)

fig.tight_layout()
fig.savefig(os.path.join(fig_dir, 'fig_summary.pdf'), bbox_inches='tight')
fig.savefig(os.path.join(fig_dir, 'fig_summary.png'), bbox_inches='tight')
plt.close()
print("fig_summary saved")

print("\nAll figures generated successfully.")
