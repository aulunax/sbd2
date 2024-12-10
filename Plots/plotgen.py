import matplotlib.pyplot as plt

# plot 1
D_values = [2, 4, 6, 10, 20, 50]
block_reads = [68451, 49086, 39877, 30853, 21809, 12230]
block_writes = [30452, 24137, 21114, 18198, 15135, 11838]
total_records = [9992, 9996, 9997, 9994, 9996, 9997]

avg_operations_per_record = [(block_reads[i] + block_writes[i]) / total_records[i] for i in range(len(D_values))]
print('avg_operations_per_record:', avg_operations_per_record)

plt.figure(figsize=(10, 6))
plt.plot(D_values, avg_operations_per_record, marker='o', label='Średnia liczba operacji blokowych na rekord')

plt.title('Średnia liczba operacji blokowych na rekord w zależności od wartości D\n (Liczba wstawionych rekordów=10000)')
plt.xlabel('Wartość D')
plt.ylabel('Średnia liczba operacji na rekord')
plt.legend()
plt.grid(True)

plt.savefig('Plots/block_operations_per_record.png')


# plot 2
# size calc
page_sizes = [(12 + 24 * d) for d in D_values]
total_pages_count = [3080, 1514, 1003, 600, 294, 119]


record_sizes = [total_records[i]*12 for i in range(len(D_values))]
print('record_sizes:', record_sizes)

total_tree_size = [page_sizes[i] * total_pages_count[i] for i in range(len(D_values))]
print('total_tree_size:', total_tree_size)

filled_tree = [record_sizes[i] / total_tree_size[i] for i in range(len(D_values))]
print('filled_tree:', filled_tree)

plt.figure(figsize=(10, 6))
plt.plot(D_values, filled_tree, marker='o', label='Wypełnienie drzewa')
plt.title('Wypełnienie pliku indeksowego w zależności od wartości D\n (Liczba wstawionych rekordów=10000)')
plt.xlabel('Wartość D')
plt.ylabel('Wypełnienie drzewa')
plt.ylim(0.5, 1)
plt.legend()
plt.grid(True)

plt.savefig('Plots/tree_fill.png')

# plot 3
record_counts = [250, 1000, 2500, 10000, 25000]

# data for d=2
block_reads_d2 = [943, 5151, 14305, 68342, 185710]
block_writes_d2 = [655, 2894, 7370, 30203, 75558]
total_operations_d2 = [block_reads_d2[i] + block_writes_d2[i] for i in range(len(record_counts))]
print('total_operations_d2:', total_operations_d2)

# data for d=10
block_reads_d10 = [222, 1632, 6050, 30458, 87358]
block_writes_d10 = [250, 1435, 4076, 18191, 46218]
total_operations_d10 = [block_reads_d10[i] + block_writes_d10[i] for i in range(len(record_counts))]
print('total_operations_d10:', total_operations_d10)

plt.figure(figsize=(10, 6))
plt.plot(record_counts, total_operations_d2, marker='o', label='Operacje blokowe na pliku indeksowym (D=2)')
plt.plot(record_counts, total_operations_d10, marker='o', label='Operacje blokowe na pliku indeksowym (D=10)')

plt.xscale('log')
plt.title('Liczba operacji dyskowych w zależności od liczby wstawionych rekordów dla D=2 i D=10')
plt.xlabel('Liczba wstawionych rekordów (log scale)')
plt.ylabel('Liczba operacji dyskowych')
plt.legend()
plt.grid(True)

plt.savefig('Plots/disk_operations_by_record_count.png')


